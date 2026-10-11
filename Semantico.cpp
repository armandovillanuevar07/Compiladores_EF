#include "Semantico.h"
#include "Postfija.h"

void AnalizadorSemantico::registrarError(const Token &t, const string &mensaje)
{
    ErrorSemantico e;
    e.linea = t.linea;
    e.columna = t.columna;
    e.mensaje = mensaje;
    errores.push_back(e);
}

bool AnalizadorSemantico::compatible(TipoDato destino, TipoDato origen)
{
    if (destino == origen)
    {
        return true;
    }

    // un int puede guardarse en un float (ampliacion sin perdida)
    if (destino == TipoDato::REAL && origen == TipoDato::ENTERO)
    {
        return true;
    }

    return false;
}

TipoDato AnalizadorSemantico::tipoExpresion(const vector<Token> &postfija)
{
    vector<TipoDato> pila;

    for (const Token &t : postfija)
    {
        if (t.tipo == TipoToken::LITERAL_ENTERO)
        {
            pila.push_back(TipoDato::ENTERO);
        }
        else if (t.tipo == TipoToken::LITERAL_REAL)
        {
            pila.push_back(TipoDato::REAL);
        }
        else if (t.tipo == TipoToken::LITERAL_STRING)
        {
            pila.push_back(TipoDato::CADENA);
        }
        else if (t.tipo == TipoToken::IDENTIFICADOR)
        {
            // el identificador usado como valor debe estar declarado
            const Simbolo *s = tabla.buscar(t.lexema);

            if (s == nullptr)
            {
                registrarError(t, "Identificador '" + t.lexema + "' no declarado");
                pila.push_back(TipoDato::INVALIDO);
            }
            else
            {
                pila.push_back(s->tipo);
            }
        }
        else if (t.tipo == TipoToken::OPERADOR)
        {
            if (pila.size() < 2)
            {
                return TipoDato::INVALIDO;
            }

            TipoDato b = pila.back();
            pila.pop_back();
            TipoDato a = pila.back();
            pila.pop_back();

            if (a == TipoDato::INVALIDO || b == TipoDato::INVALIDO)
            {
                // el error ya fue reportado antes
                pila.push_back(TipoDato::INVALIDO);
            }
            else if (a == TipoDato::CADENA || b == TipoDato::CADENA)
            {
                registrarError(t, "El operador '" + t.lexema +
                                      "' no se puede aplicar a operandos de tipo string");
                pila.push_back(TipoDato::INVALIDO);
            }
            else if (a == TipoDato::REAL || b == TipoDato::REAL)
            {
                pila.push_back(TipoDato::REAL);
            }
            else
            {
                pila.push_back(TipoDato::ENTERO);
            }
        }
    }

    if (pila.empty())
    {
        return TipoDato::INVALIDO;
    }
    return pila.back();
}

void AnalizadorSemantico::analizarSentencia(const Sentencia &s)
{
    size_t erroresAntes = errores.size();
    TipoDato destino = TipoDato::INVALIDO;

    if (s.esDeclaracion)
    {
        destino = tipoDesdeTexto(s.tipo);

        // variable declarada dos veces
        const Simbolo *previo = tabla.buscar(s.id.lexema);
        if (previo != nullptr)
        {
            registrarError(s.id, "Variable '" + s.id.lexema +
                                     "' ya fue declarada en la linea " +
                                     to_string(previo->linea) +
                                     " (tipo " + nombreTipo(previo->tipo) + ")");
        }
    }
    else
    {
        // asignacion: la variable destino debe existir
        const Simbolo *simbolo = tabla.buscar(s.id.lexema);
        if (simbolo == nullptr)
        {
            registrarError(s.id, "Identificador '" + s.id.lexema + "' no declarado");
        }
        else
        {
            destino = simbolo->tipo;
        }
    }

    // tipo del valor asignado (se evalua ANTES de registrar la variable,
    // por eso "int x = x + 1;" reporta que x no esta declarada)
    TipoDato origen = tipoExpresion(aPostfija(s.expresion));

    if (destino != TipoDato::INVALIDO &&
        origen != TipoDato::INVALIDO &&
        !compatible(destino, origen))
    {
        string mensaje = "Incompatibilidad de tipos: no se puede asignar un valor '" +
                         nombreTipo(origen) + "' a la variable '" + s.id.lexema +
                         "' de tipo '" + nombreTipo(destino) + "'";

        if (destino == TipoDato::ENTERO && origen == TipoDato::REAL)
        {
            mensaje += " (perdida de precision)";
        }

        registrarError(s.id, mensaje);
    }

    // la variable solo se registra si la declaracion es valida
    if (errores.size() == erroresAntes)
    {
        if (s.esDeclaracion)
        {
            tabla.insertar(s.id.lexema, destino, s.id.linea);
        }
        validas.push_back(s);
    }
}

void AnalizadorSemantico::analizar(const vector<Sentencia> &sentencias)
{
    for (const Sentencia &s : sentencias)
    {
        analizarSentencia(s);
    }
}

const TablaSimbolos &AnalizadorSemantico::obtenerTabla() const
{
    return tabla;
}

const vector<ErrorSemantico> &AnalizadorSemantico::obtenerErrores() const
{
    return errores;
}

const vector<Sentencia> &AnalizadorSemantico::obtenerSentenciasValidas() const
{
    return validas;
}
