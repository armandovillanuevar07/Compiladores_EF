#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include "Lexer.h"
#include "Parser.h"
#include "Semantico.h"
#include "Postfija.h"
#include "CodigoIntermedio.h"

using namespace std;

string nombreToken(TipoToken tipo)
{
    switch (tipo)
    {
    case TipoToken::LITERAL_ENTERO:
        return "LITERAL_ENTERO";
    case TipoToken::LITERAL_REAL:
        return "LITERAL_REAL";
    case TipoToken::LITERAL_STRING:
        return "LITERAL_STRING";
    case TipoToken::IDENTIFICADOR:
        return "IDENTIFICADOR";
    case TipoToken::PALABRA_CLAVE:
        return "PALABRA_CLAVE";
    case TipoToken::OPERADOR:
        return "OPERADOR";
    case TipoToken::ASIGNACION:
        return "ASIGNACION";
    case TipoToken::DELIMITADOR:
        return "DELIMITADOR";
    case TipoToken::DESCONOCIDO:
        return "DESCONOCIDO";
    }
    return "DESCONOCIDO";
}

void titulo(const string &texto)
{
    cout << "\n==================================================\n";
    cout << texto << "\n";
    cout << "==================================================\n";
}

// Lee el programa fuente (varias lineas).
//  - Con argumento:  ./compilador archivo.txt
//  - Sin argumento:  se escribe el codigo y se termina con una linea "FIN"
//                    (o con Ctrl+D)
bool leerCodigo(int argc, char *argv[], string &codigo)
{
    if (argc > 1)
    {
        ifstream archivo(argv[1]);
        if (!archivo.is_open())
        {
            cerr << "No se pudo abrir el archivo: " << argv[1] << endl;
            return false;
        }

        stringstream buffer;
        buffer << archivo.rdbuf();
        codigo = buffer.str();
        return true;
    }

    cout << "Ingrese el codigo fuente (varias lineas).\n";
    cout << "Termine con una linea que contenga solo FIN (o Ctrl+D):\n";

    string linea;
    codigo = "";
    while (getline(cin, linea))
    {
        if (linea == "FIN")
        {
            break;
        }
        codigo += linea + "\n";
    }
    return true;
}

void mostrarCodigoFuente(const string &codigo)
{
    titulo("CODIGO FUENTE");

    stringstream ss(codigo);
    string linea;
    int numero = 1;

    while (getline(ss, linea))
    {
        if (!linea.empty() && linea.back() == '\r')
        {
            linea.pop_back();
        }
        cout << numero << " | " << linea << "\n";
        numero++;
    }
}

int main(int argc, char *argv[])
{
    string codigo;
    if (!leerCodigo(argc, argv, codigo))
    {
        return 1;
    }

    mostrarCodigoFuente(codigo);

    // ---------------------------------
    // 1. ANALISIS LEXICO
    // ---------------------------------
    Lexer lexer(codigo);
    vector<Token> tokens = lexer.analizar();
    vector<ErrorLexico> erroresLexicos = lexer.obtenerErrores();

    titulo("1. ANALISIS LEXICO - TOKENS");
    for (const Token &t : tokens)
    {
        cout << "<" << nombreToken(t.tipo) << ", " << t.lexema << ">"
             << "   [linea " << t.linea << ", col " << t.columna << "]\n";
    }

    cout << "\n--- Lista de errores lexicos (" << erroresLexicos.size() << ") ---\n";
    if (erroresLexicos.empty())
    {
        cout << "Sin errores lexicos.\n";
    }
    for (const ErrorLexico &e : erroresLexicos)
    {
        cout << "[Linea " << e.linea << ", Col " << e.columna << "] "
             << e.mensaje << ": '" << e.lexema << "'\n";
    }

    // ---------------------------------
    // 2. ANALISIS SINTACTICO
    // ---------------------------------
    Parser parser(tokens);
    parser.analizar();
    vector<ErrorSintactico> erroresSintacticos = parser.obtenerErrores();
    vector<Sentencia> sentencias = parser.obtenerSentencias();

    titulo("2. ANALISIS SINTACTICO");
    cout << "Sentencias sintacticamente correctas: " << sentencias.size() << "\n";
    cout << "\n--- Errores sintacticos (" << erroresSintacticos.size() << ") ---\n";
    if (erroresSintacticos.empty())
    {
        cout << "Sin errores sintacticos.\n";
    }
    for (const ErrorSintactico &e : erroresSintacticos)
    {
        cout << "[Linea " << e.linea << ", Col " << e.columna << "] Error sintactico: "
             << e.mensaje << "\n";
    }

    // ---------------------------------
    // 3. ANALISIS SEMANTICO
    // ---------------------------------
    AnalizadorSemantico semantico;
    semantico.analizar(sentencias);

    titulo("3. ANALISIS SEMANTICO");
    cout << "--- Tabla de simbolos ---\n";
    const vector<Simbolo> &simbolos = semantico.obtenerTabla().obtener();
    if (simbolos.empty())
    {
        cout << "(vacia)\n";
    }
    for (const Simbolo &s : simbolos)
    {
        cout << s.nombre << " -> " << nombreTipo(s.tipo)
             << "   (declarada en linea " << s.linea << ")\n";
    }

    const vector<ErrorSemantico> &erroresSemanticos = semantico.obtenerErrores();
    cout << "\n--- Errores semanticos (" << erroresSemanticos.size() << ") ---\n";
    if (erroresSemanticos.empty())
    {
        cout << "Sin errores semanticos.\n";
    }
    for (const ErrorSemantico &e : erroresSemanticos)
    {
        cout << "[Linea " << e.linea << ", Col " << e.columna << "] Error semantico: "
             << e.mensaje << "\n";
    }

    // ---------------------------------
    // 4. CODIGO INTERMEDIO (tres direcciones)
    // ---------------------------------
    const vector<Sentencia> &validas = semantico.obtenerSentenciasValidas();

    set<string> nombresVariables;
    for (const Simbolo &s : simbolos)
    {
        nombresVariables.insert(s.nombre);
    }

    GeneradorCodigo generador(nombresVariables);

    titulo("4. CODIGO INTERMEDIO");
    cout << "(se genera codigo solo para las sentencias sin errores)\n\n";
    cout << "--- Conversion a notacion postfija ---\n";
    if (validas.empty())
    {
        cout << "(no hay sentencias validas)\n";
    }
    for (const Sentencia &s : validas)
    {
        cout << "Linea " << s.id.linea << ":  " << s.id.lexema << " = "
             << tokensATexto(s.expresion) << "   =>   postfija: "
             << tokensATexto(aPostfija(s.expresion)) << "\n";
        generador.generar(s);
    }

    const vector<Instruccion> &intermedio = generador.obtenerCodigo();
    cout << "\n--- Codigo de tres direcciones ---\n";
    if (intermedio.empty())
    {
        cout << "(vacio)\n";
    }
    for (const Instruccion &ins : intermedio)
    {
        cout << instruccionATexto(ins) << "\n";
    }

    // ---------------------------------
    // 5. OPTIMIZACION
    // ---------------------------------
    vector<Instruccion> optimizado = optimizar(intermedio, generador.obtenerTemporales());

    titulo("5. OPTIMIZACION");
    cout << "--- ANTES (" << intermedio.size() << " instruccion(es)) ---\n";
    for (const Instruccion &ins : intermedio)
    {
        cout << instruccionATexto(ins) << "\n";
    }

    cout << "\n--- DESPUES (" << optimizado.size() << " instruccion(es)) ---\n";
    for (const Instruccion &ins : optimizado)
    {
        cout << instruccionATexto(ins) << "\n";
    }

    // ---------------------------------
    // RESUMEN
    // ---------------------------------
    size_t totalErrores = erroresLexicos.size() + erroresSintacticos.size() + erroresSemanticos.size();

    titulo("RESUMEN");
    cout << "Errores lexicos    : " << erroresLexicos.size() << "\n";
    cout << "Errores sintacticos: " << erroresSintacticos.size() << "\n";
    cout << "Errores semanticos : " << erroresSemanticos.size() << "\n";
    cout << (totalErrores == 0 ? "Resultado: compilacion exitosa\n"
                               : "Resultado: compilacion con errores\n");

    return totalErrores == 0 ? 0 : 1;
}
