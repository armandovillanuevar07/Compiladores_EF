#include "Lexer.h"
#include <cctype>

Lexer::Lexer(string _codigo)
{
    codigo = _codigo;
    posicion = 0;
    linea = 1;
    columna = 1;
}

bool Lexer::esFinal()
{
    return posicion >= (int)codigo.length();
}

char Lexer::actual()
{
    if (esFinal())
    {
        return '\0';
    }
    return codigo[posicion];
}

char Lexer::avanzar()
{
    char caracter = codigo[posicion];
    posicion++;

    // controla salto de linea
    if (caracter == '\n')
    {
        linea++;
        columna = 1;
    }
    // los bytes de continuacion UTF-8 (tildes, ñ) no cuentan como columna
    else if (((unsigned char)caracter & 0xC0) != 0x80)
    {
        columna++;
    }

    return caracter;
}

void Lexer::ignorarEspacios()
{
    while (!esFinal() && isspace((unsigned char)actual()))
    {
        avanzar();
    }
}

// ---------------------------------
// Literal entero o real
// ---------------------------------
Token Lexer::reconocerNumero()
{
    string lexema = "";

    int lineaInicio = linea;
    int columnaInicio = columna;

    bool tienePunto = false;

    // parte entera
    while (!esFinal() && isdigit((unsigned char)actual()))
    {
        lexema += avanzar();
    }

    // parte decimal: el punto debe estar seguido de al menos un digito
    if (actual() == '.' &&
        posicion + 1 < (int)codigo.length() &&
        isdigit((unsigned char)codigo[posicion + 1]))
    {
        tienePunto = true;
        lexema += avanzar(); // '.'

        while (!esFinal() && isdigit((unsigned char)actual()))
        {
            lexema += avanzar();
        }
    }

    // numero mal formado: 12abc, 5., 1.2.3 ...
    if (actual() == '.' || isalpha((unsigned char)actual()) || actual() == '_')
    {
        while (!esFinal() &&
               (isalnum((unsigned char)actual()) ||
                actual() == '_' ||
                actual() == '.'))
        {
            lexema += avanzar();
        }

        registrarError(lexema, lineaInicio, columnaInicio,
                       "Literal numerico mal formado");

        return {
            TipoToken::DESCONOCIDO,
            lexema,
            lineaInicio,
            columnaInicio};
    }

    if (tienePunto)
    {
        return {
            TipoToken::LITERAL_REAL,
            lexema,
            lineaInicio,
            columnaInicio};
    }

    return {
        TipoToken::LITERAL_ENTERO,
        lexema,
        lineaInicio,
        columnaInicio};
}

// ---------------------------------
// Literal string: "texto"
// Debe ir obligatoriamente entre comillas dobles y en una sola linea.
// "10.5" es un string (no un float) y "1" es un string (no un int).
// ---------------------------------
Token Lexer::reconocerString()
{
    int lineaInicio = linea;
    int columnaInicio = columna;

    string lexema = "";
    lexema += avanzar(); // comilla de apertura

    while (!esFinal() && actual() != '"' && actual() != '\n')
    {
        lexema += avanzar();
    }

    // comilla de cierre encontrada
    if (!esFinal() && actual() == '"')
    {
        lexema += avanzar();

        return {
            TipoToken::LITERAL_STRING,
            lexema,
            lineaInicio,
            columnaInicio};
    }

    // llego al fin de linea o de archivo sin cerrar
    while (!lexema.empty() && lexema.back() == '\r')
    {
        lexema.pop_back();
    }

    registrarError(lexema, lineaInicio, columnaInicio,
                   "String sin cerrar: falta la comilla doble de cierre");

    return {
        TipoToken::DESCONOCIDO,
        lexema,
        lineaInicio,
        columnaInicio};
}

Token Lexer::reconocerIdentificador()
{
    string lexema = "";

    int lineaInicio = linea;
    int columnaInicio = columna;

    while (!esFinal() &&
           (isalnum((unsigned char)actual()) || actual() == '_'))
    {
        lexema += avanzar();
    }

    if (esPalabraClave(lexema))
    {
        return {
            TipoToken::PALABRA_CLAVE,
            lexema,
            lineaInicio,
            columnaInicio};
    }

    return {
        TipoToken::IDENTIFICADOR,
        lexema,
        lineaInicio,
        columnaInicio};
}

Token Lexer::reconocerOperador()
{
    string lexema = "";

    int lineaInicio = linea;
    int columnaInicio = columna;

    lexema += avanzar();

    return {
        TipoToken::OPERADOR,
        lexema,
        lineaInicio,
        columnaInicio};
}

bool Lexer::esPalabraClave(string palabra)
{
    return palabra == "int" || palabra == "float" || palabra == "string" || palabra == "if" || palabra == "else";
}

// error lexico
vector<ErrorLexico> Lexer::obtenerErrores()
{
    return errores;
}

void Lexer::registrarError(string lexema, int lineaError, int columnaError, string mensaje)
{
    ErrorLexico error;
    error.lexema = lexema;
    error.linea = lineaError;
    error.columna = columnaError;
    error.mensaje = mensaje;
    errores.push_back(error);
}

vector<Token> Lexer::analizar()
{
    vector<Token> tokens;
    while (!esFinal())
    {
        ignorarEspacios();
        if (esFinal())
        {
            break;
        }
        char c = actual();

        // Literal entero o real
        if (isdigit((unsigned char)c))
        {
            tokens.push_back(reconocerNumero());
        }

        // Literal string (comillas dobles)
        else if (c == '"')
        {
            tokens.push_back(reconocerString());
        }

        // Identificador o palabra clave
        else if (isalpha((unsigned char)c) || c == '_')
        {
            tokens.push_back(reconocerIdentificador());
        }

        // Operadores aritméticos
        else if (c == '+' || c == '-' || c == '*' || c == '/')
        {
            tokens.push_back(reconocerOperador());
        }

        // Asignación
        else if (c == '=')
        {
            int lineaInicio = linea;
            int columnaInicio = columna;

            string lexema = "";
            lexema += avanzar();

            tokens.push_back({TipoToken::ASIGNACION,
                              lexema,
                              lineaInicio,
                              columnaInicio});
        }

        // Delimitadores: ; ( )
        else if (c == ';' || c == '(' || c == ')')
        {
            int lineaInicio = linea;
            int columnaInicio = columna;

            string lexema = "";
            lexema += avanzar();

            tokens.push_back({TipoToken::DELIMITADOR,
                              lexema,
                              lineaInicio,
                              columnaInicio});
        }

        // Carácter desconocido - error lexico (el analisis continua)
        else
        {
            int lineaError = linea;
            int columnaError = columna;

            string lexema = "";
            lexema += avanzar();

            // un caracter UTF-8 de varios bytes (ñ, tildes) se toma completo
            while (!esFinal() && (((unsigned char)actual()) & 0xC0) == 0x80)
            {
                lexema += avanzar();
            }

            registrarError(lexema, lineaError, columnaError, "Caracter no reconocido");
            tokens.push_back({TipoToken::DESCONOCIDO,
                              lexema,
                              lineaError,
                              columnaError});
        }
    }
    return tokens;
}
