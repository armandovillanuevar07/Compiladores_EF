#ifndef POSTFIJA_H
#define POSTFIJA_H

#include <vector>
#include <string>
#include "Token.h"

using namespace std;

bool esOperando(const Token &t);
bool esOperador(const Token &t);
int precedencia(const string &op);

// Convierte una expresion en notacion infija (ya validada por el Parser)
// a notacion postfija, usando una pila de operadores (algoritmo de Dijkstra).
//   (a + b) * c   ->   a b + c *
vector<Token> aPostfija(const vector<Token> &infija);

// Texto de una lista de tokens separados por espacio (para mostrar en pantalla)
string tokensATexto(const vector<Token> &tokens);

#endif
