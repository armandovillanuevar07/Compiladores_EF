#include "Parser.h"

using namespace std;

// Constructor
Parser::Parser(vector<Token> tokens)
{
    this->tokens = tokens;
    posicion = 0;
}

// Verifica si terminamos de analizar
bool Parser::esFinal()
{
    return posicion >= (int)tokens.size();
}

// Obtiene el token actual
Token Parser::actual()
{
    return tokens[posicion];
}

// Comprueba si el token actual
// corresponde al tipo esperado
bool Parser::coincidir(TipoToken tipoEsperado)
{
    if (!esFinal() &&
        actual().tipo == tipoEsperado)
    {
        posicion++;
        return true;
    }

    return false;
}

// Igual que coincidir, pero ademas exige un lexema concreto
bool Parser::coincidirLexema(TipoToken tipoEsperado, string lexema)
{
    if (!esFinal() &&
        actual().tipo == tipoEsperado &&
        actual().lexema == lexema)
    {
        posicion++;
        return true;
    }

    return false;
}

// ---------------------------------
// Tipo → int | float | string
// ---------------------------------

bool Parser::tipo()
{
    if (esFinal())
    {
        return false;
    }

    if (actual().tipo == TipoToken::PALABRA_CLAVE)
    {
        if (actual().lexema == "int" ||
            actual().lexema == "float" ||
            actual().lexema == "string")
        {
            posicion++;
            return true;
        }
    }
    return false;
}

// ---------------------------------
// Valor → NUM | REAL | STRING | ID
// ---------------------------------

bool Parser::valor()
{
    if (esFinal())
    {
        return false;
    }

    if (coincidir(TipoToken::LITERAL_ENTERO))
    {
        return true;
    }

    if (coincidir(TipoToken::LITERAL_REAL))
    {
        return true;
    }

    if (coincidir(TipoToken::LITERAL_STRING))
    {
        return true;
    }

    if (coincidir(TipoToken::IDENTIFICADOR))
    {
        return true;
    }

    return false;
}

// ---------------------------------
// Factor → Valor | ( Expresion )
// ---------------------------------

bool Parser::factor()
{
    if (valor())
    {
        return true;
    }

    if (coincidirLexema(TipoToken::DELIMITADOR, "("))
    {
        if (!expresion())
        {
            return false;
        }

        if (!coincidirLexema(TipoToken::DELIMITADOR, ")"))
        {
            error("')'");
            return false;
        }

        return true;
    }

    error("NUM, REAL, STRING, IDENTIFICADOR o '('");
    return false;
}

// ---------------------------------
// Termino → Factor { (* | /) Factor }
// ---------------------------------

bool Parser::termino()
{
    if (!factor())
    {
        return false;
    }

    while (!esFinal() &&
           actual().tipo == TipoToken::OPERADOR &&
           (actual().lexema == "*" || actual().lexema == "/"))
    {
        posicion++;

        if (!factor())
        {
            return false;
        }
    }

    return true;
}

// ---------------------------------
// Expresion → Termino { (+ | -) Termino }
// ---------------------------------

bool Parser::expresion()
{
    if (!termino())
    {
        return false;
    }

    while (!esFinal() &&
           actual().tipo == TipoToken::OPERADOR &&
           (actual().lexema == "+" || actual().lexema == "-"))
    {
        posicion++;

        if (!termino())
        {
            return false;
        }
    }

    return true;
}

// ---------------------------------
// Declaracion → Tipo ID = Expresion ;
// Asignacion  → ID = Expresion ;
// ---------------------------------

bool Parser::sentencia()
{
    Sentencia s;

    if (actual().tipo == TipoToken::PALABRA_CLAVE &&
        (actual().lexema == "int" ||
         actual().lexema == "float" ||
         actual().lexema == "string"))
    {
        // Declaracion
        s.esDeclaracion = true;
        s.tipo = actual().lexema;
        tipo();

        // ID
        if (esFinal() || actual().tipo != TipoToken::IDENTIFICADOR)
        {
            error("IDENTIFICADOR");
            return false;
        }
        s.id = actual();
        posicion++;
    }
    else if (actual().tipo == TipoToken::IDENTIFICADOR)
    {
        // Asignacion
        s.esDeclaracion = false;
        s.tipo = "";
        s.id = actual();
        posicion++;
    }
    else
    {
        error("int, float, string o un IDENTIFICADOR");
        return false;
    }

    // =
    if (!coincidir(TipoToken::ASIGNACION))
    {
        error("'='");
        return false;
    }

    // Expresion
    int inicioExpresion = posicion;

    if (!expresion())
    {
        return false; // el error ya fue reportado dentro de factor()
    }

    s.expresion = vector<Token>(tokens.begin() + inicioExpresion,
                                tokens.begin() + posicion);

    // ;
    if (!coincidirLexema(TipoToken::DELIMITADOR, ";"))
    {
        error("';'");
        return false;
    }

    sentencias.push_back(s);
    return true;
}

// ---------------------------------
// Recuperacion de errores
// Salta tokens hasta consumir el ';' que cierra la sentencia con error,
// o hasta encontrar el inicio de una nueva declaracion (int/float/string).
// ---------------------------------

void Parser::sincronizar()
{
    if (esFinal())
    {
        return;
    }

    // el error ocurrio justo en un ';'
    if (actual().tipo == TipoToken::DELIMITADOR && actual().lexema == ";")
    {
        posicion++;
        return;
    }

    // el error ocurrio al inicio de una nueva declaracion (falto el ';')
    if (actual().tipo == TipoToken::PALABRA_CLAVE &&
        (actual().lexema == "int" || actual().lexema == "float" || actual().lexema == "string"))
    {
        return;
    }

    // saltamos el token que causo el error
    posicion++;

    while (!esFinal())
    {
        if (actual().tipo == TipoToken::DELIMITADOR && actual().lexema == ";")
        {
            posicion++;
            return;
        }

        if (actual().tipo == TipoToken::PALABRA_CLAVE &&
            (actual().lexema == "int" || actual().lexema == "float" || actual().lexema == "string"))
        {
            return;
        }

        posicion++;
    }
}

// ---------------------------------
// Iniciar análisis sintáctico
// Programa → Sentencia*
// ---------------------------------

bool Parser::analizar()
{
    while (!esFinal())
    {
        if (!sentencia())
        {
            sincronizar();
        }
    }

    return errores.empty();
}

// Registra un error sintactico (no se imprime aqui: main los muestra juntos)
void Parser::error(string esperado)
{
    ErrorSintactico e;

    if (!esFinal())
    {
        e.linea = actual().linea;
        e.columna = actual().columna;
        e.mensaje = "se esperaba " + esperado +
                    ", pero se encontro '" + actual().lexema + "'";
    }
    else
    {
        // fin de la entrada: se senala la posicion justo despues del ultimo token
        if (tokens.empty())
        {
            e.linea = 1;
            e.columna = 1;
        }
        else
        {
            e.linea = tokens.back().linea;
            e.columna = tokens.back().columna + (int)tokens.back().lexema.length();
        }
        e.mensaje = "se esperaba " + esperado +
                    ", pero se llego al final de la entrada";
    }

    errores.push_back(e);
}

vector<ErrorSintactico> Parser::obtenerErrores()
{
    return errores;
}

vector<Sentencia> Parser::obtenerSentencias()
{
    return sentencias;
}
