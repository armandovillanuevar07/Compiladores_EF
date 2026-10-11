#ifndef SENTENCIA_H
#define SENTENCIA_H

#include <vector>
#include <string>
#include "Token.h"

using namespace std;

// Una sentencia reconocida por el Parser:
//   Declaracion -> Tipo ID = Expresion ;
//   Asignacion  -> ID = Expresion ;
// La expresion se guarda como la lista de tokens en notacion infija,
// para que las fases siguientes (semantica y codigo intermedio) la usen.
struct Sentencia
{
    bool esDeclaracion;     // true: "int x = ...;"   false: "x = ...;"
    string tipo;            // int | float | string (solo en declaraciones)
    Token id;               // variable destino
    vector<Token> expresion; // expresion en notacion infija
};

#endif
