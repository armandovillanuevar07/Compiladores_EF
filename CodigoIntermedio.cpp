#include "CodigoIntermedio.h"
#include "Postfija.h"
#include <cctype>
#include <sstream>
#include <iomanip>

string instruccionATexto(const Instruccion &ins)
{
    if (ins.op.empty())
    {
        return ins.resultado + " = " + ins.arg1;
    }
    return ins.resultado + " = " + ins.arg1 + " " + ins.op + " " + ins.arg2;
}

// ---------------------------------
// Generacion de codigo de tres direcciones
// ---------------------------------

GeneradorCodigo::GeneradorCodigo(const set<string> &nombresVariables)
{
    contadorTemporales = 0;
    reservados = nombresVariables;
}

string GeneradorCodigo::nuevoTemporal()
{
    string nombre;

    do
    {
        contadorTemporales++;
        nombre = "t" + to_string(contadorTemporales);
    } while (reservados.count(nombre) > 0);

    temporales.insert(nombre);
    return nombre;
}

void GeneradorCodigo::generar(const Sentencia &s)
{
    // la expresion infija se pasa a postfija y luego se recorre con una pila
    vector<Token> postfija = aPostfija(s.expresion);
    vector<string> pila;

    for (const Token &t : postfija)
    {
        if (esOperando(t))
        {
            pila.push_back(t.lexema);
        }
        else if (esOperador(t))
        {
            if (pila.size() < 2)
            {
                return; // expresion mal formada (no deberia ocurrir tras el Parser)
            }

            string b = pila.back();
            pila.pop_back();
            string a = pila.back();
            pila.pop_back();

            string temporal = nuevoTemporal();
            codigo.push_back({temporal, a, t.lexema, b});
            pila.push_back(temporal);
        }
    }

    if (pila.empty())
    {
        return;
    }

    // asignacion final:  variable = resultado de la expresion
    codigo.push_back({s.id.lexema, pila.back(), "", ""});
}

const vector<Instruccion> &GeneradorCodigo::obtenerCodigo() const
{
    return codigo;
}

const set<string> &GeneradorCodigo::obtenerTemporales() const
{
    return temporales;
}

// ---------------------------------
// Optimizacion
// ---------------------------------

static bool esNumero(const string &s)
{
    if (s.empty())
    {
        return false;
    }

    size_t i = (s[0] == '-') ? 1 : 0;
    return i < s.size() && isdigit((unsigned char)s[i]);
}

static string formatearReal(double valor)
{
    ostringstream ss;
    ss << setprecision(10) << valor;
    string texto = ss.str();

    // un real siempre debe verse como real: 5 -> 5.0
    if (texto.find('.') == string::npos &&
        texto.find('e') == string::npos &&
        texto.find("inf") == string::npos &&
        texto.find("nan") == string::npos)
    {
        texto += ".0";
    }
    return texto;
}

// Calcula "a op b" cuando ambos son constantes. Devuelve false si no se puede
// (division entre cero, operador desconocido, numero fuera de rango).
static bool plegar(const string &a, const string &op, const string &b, string &resultado)
{
    try
    {
        bool real = (a.find('.') != string::npos) || (b.find('.') != string::npos);

        if (real)
        {
            double x = stod(a);
            double y = stod(b);
            double r;

            if (op == "+")
                r = x + y;
            else if (op == "-")
                r = x - y;
            else if (op == "*")
                r = x * y;
            else if (op == "/" && y != 0.0)
                r = x / y;
            else
                return false;

            resultado = formatearReal(r);
            return true;
        }

        long long x = stoll(a);
        long long y = stoll(b);
        long long r;

        if (op == "+")
            r = x + y;
        else if (op == "-")
            r = x - y;
        else if (op == "*")
            r = x * y;
        else if (op == "/" && y != 0)
            r = x / y;
        else
            return false;

        resultado = to_string(r);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

static int contarUsos(const vector<Instruccion> &codigo, const string &nombre)
{
    int usos = 0;
    for (const Instruccion &ins : codigo)
    {
        if (ins.arg1 == nombre)
        {
            usos++;
        }
        if (ins.arg2 == nombre)
        {
            usos++;
        }
    }
    return usos;
}

vector<Instruccion> optimizar(const vector<Instruccion> &original,
                              const set<string> &temporales)
{
    vector<Instruccion> codigo = original;

    bool cambio = true;
    while (cambio)
    {
        cambio = false;

        // 1. Plegado de constantes: t1 = 2 + 3  ->  t1 = 5
        for (Instruccion &ins : codigo)
        {
            if (!ins.op.empty() && esNumero(ins.arg1) && esNumero(ins.arg2))
            {
                string r;
                if (plegar(ins.arg1, ins.op, ins.arg2, r))
                {
                    ins.arg1 = r;
                    ins.op = "";
                    ins.arg2 = "";
                    cambio = true;
                }
            }
        }

        // 2. Propagacion: si t1 = 5, se reemplaza t1 por 5 donde se use
        for (size_t i = 0; i < codigo.size(); i++)
        {
            if (temporales.count(codigo[i].resultado) > 0 &&
                codigo[i].op.empty() &&
                esNumero(codigo[i].arg1))
            {
                string temporal = codigo[i].resultado;
                string valor = codigo[i].arg1;

                for (size_t j = i + 1; j < codigo.size(); j++)
                {
                    if (codigo[j].arg1 == temporal)
                    {
                        codigo[j].arg1 = valor;
                    }
                    if (codigo[j].arg2 == temporal)
                    {
                        codigo[j].arg2 = valor;
                    }
                }

                codigo.erase(codigo.begin() + i);
                cambio = true;
                break; // reiniciamos: pueden aparecer nuevas constantes
            }
        }
    }

    // 3. Copias redundantes:  t2 = t1 * c ; resultado = t2  ->  resultado = t1 * c
    size_t i = 1;
    while (i < codigo.size())
    {
        if (codigo[i].op.empty() &&
            temporales.count(codigo[i].arg1) > 0 &&
            codigo[i - 1].resultado == codigo[i].arg1 &&
            contarUsos(codigo, codigo[i].arg1) == 1)
        {
            codigo[i - 1].resultado = codigo[i].resultado;
            codigo.erase(codigo.begin() + i);
        }
        else
        {
            i++;
        }
    }

    return codigo;
}
