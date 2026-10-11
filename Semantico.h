#ifndef SEMANTICO_H
#define SEMANTICO_H

#include <vector>
#include <string>
#include "Token.h"
#include "Sentencia.h"
#include "TablaSimbolos.h"

using namespace std;

struct ErrorSemantico
{
    int linea;
    int columna;
    string mensaje;
};

// Primera capa semantica:
//  - tabla de simbolos (nombre -> tipo)
//  - variable declarada dos veces
//  - identificador usado sin declarar
//  - compatibilidad basica de tipos
class AnalizadorSemantico
{
private:
    TablaSimbolos tabla;
    vector<ErrorSemantico> errores;
    vector<Sentencia> validas; // sentencias sin errores semanticos

    void registrarError(const Token &t, const string &mensaje);

    // Calcula el tipo de una expresion en postfija usando una pila de tipos
    TipoDato tipoExpresion(const vector<Token> &postfija);

    // int <- int | float <- int, float | string <- string
    bool compatible(TipoDato destino, TipoDato origen);

    void analizarSentencia(const Sentencia &s);

public:
    void analizar(const vector<Sentencia> &sentencias);

    const TablaSimbolos &obtenerTabla() const;
    const vector<ErrorSemantico> &obtenerErrores() const;
    const vector<Sentencia> &obtenerSentenciasValidas() const;
};

#endif
