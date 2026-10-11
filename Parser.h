#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include "Token.h"
#include "Sentencia.h"

using namespace std;

// Gramatica (EBNF, sin recursividad izquierda):
//
//   Programa    -> Sentencia*
//   Sentencia   -> Declaracion | Asignacion
//   Declaracion -> Tipo ID = Expresion ;
//   Asignacion  -> ID = Expresion ;
//   Tipo        -> int | float | string
//   Expresion   -> Termino { (+ | -) Termino }
//   Termino     -> Factor  { (* | /) Factor }
//   Factor      -> Valor | ( Expresion )
//   Valor       -> NUM | REAL | STRING | ID
//
// Cuando la Expresion es un solo Valor se obtiene exactamente la regla
// minima pedida:  Declaracion -> Tipo ID = Valor ;
class Parser
{
private:
    vector<Token> tokens;
    int posicion;

    vector<ErrorSintactico> errores;
    vector<Sentencia> sentencias; // sentencias sintacticamente correctas

    // Devuelve true si ya no quedan tokens
    bool esFinal();

    // Devuelve el token que estamos analizando
    Token actual();

    // Comprueba si el token actual es del tipo esperado
    bool coincidir(TipoToken tipo);

    // Comprueba tipo y lexema (para ';' '(' ')' y operadores)
    bool coincidirLexema(TipoToken tipo, string lexema);

    // No terminales de nuestra gramática
    bool tipo();
    bool valor();
    bool factor();
    bool termino();
    bool expresion();
    bool sentencia();

    // Recuperacion de errores (modo panico): salta hasta el final de la
    // sentencia para poder seguir analizando las siguientes.
    void sincronizar();

public:
    Parser(vector<Token> tokens);

    // Inicia el análisis sintáctico de TODO el programa.
    // Devuelve true si no hubo errores sintacticos.
    bool analizar();
    void error(string esperado);

    vector<ErrorSintactico> obtenerErrores();
    vector<Sentencia> obtenerSentencias();
};

#endif
