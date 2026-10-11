#ifndef CODIGOINTERMEDIO_H
#define CODIGOINTERMEDIO_H

#include <vector>
#include <string>
#include <set>
#include "Sentencia.h"

using namespace std;

// Instruccion de tres direcciones:
//   resultado = arg1 op arg2      (op no vacio)
//   resultado = arg1              (copia: op vacio)
struct Instruccion
{
    string resultado;
    string arg1;
    string op;
    string arg2;
};

string instruccionATexto(const Instruccion &ins);

class GeneradorCodigo
{
private:
    int contadorTemporales;
    set<string> reservados;   // nombres de variables del programa
    set<string> temporales;   // temporales creados (t1, t2, ...)
    vector<Instruccion> codigo;

    string nuevoTemporal();

public:
    // 'nombresVariables': variables declaradas, para que ningun temporal
    // se llame igual que una variable del usuario (por ejemplo "t1")
    GeneradorCodigo(const set<string> &nombresVariables);

    // Genera el codigo de tres direcciones de una sentencia valida
    void generar(const Sentencia &s);

    const vector<Instruccion> &obtenerCodigo() const;
    const set<string> &obtenerTemporales() const;
};

// Optimizacion sencilla del codigo generado:
//  1. plegado de constantes:   t1 = 2 + 3      ->  t1 = 5
//  2. propagacion de constantes en temporales: resultado = t1 -> resultado = 5
//  3. eliminacion de copias redundantes de temporales:
//        t2 = t1 * c ; resultado = t2  ->  resultado = t1 * c
vector<Instruccion> optimizar(const vector<Instruccion> &codigo,
                              const set<string> &temporales);

#endif
