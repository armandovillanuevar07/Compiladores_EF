#ifndef TOKEN_H
#define TOKEN_H

#include <string>

using namespace std;

enum class TipoToken
{
    LITERAL_ENTERO,
    LITERAL_REAL,
    LITERAL_STRING, // "texto" (siempre entre comillas dobles)
    IDENTIFICADOR,
    DESCONOCIDO,

    PALABRA_CLAVE, // int, float, string, if, else
    OPERADOR,      // +, -, *, /
    ASIGNACION,    // =
    DELIMITADOR    // ; ( )
};

struct Token
{
    TipoToken tipo;
    string lexema;

    int linea;
    int columna;
};

struct ErrorLexico
{
    string lexema;

    int linea;
    int columna;

    string mensaje;
};

struct ErrorSintactico
{
    int linea;
    int columna;

    string mensaje;
};

#endif
