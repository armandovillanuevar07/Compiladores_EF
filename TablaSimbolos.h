#ifndef TABLASIMBOLOS_H
#define TABLASIMBOLOS_H

#include <string>
#include <vector>

using namespace std;

enum class TipoDato
{
    ENTERO,   // int
    REAL,     // float
    CADENA,   // string
    INVALIDO  // tipo no determinado (por un error previo)
};

string nombreTipo(TipoDato tipo);
TipoDato tipoDesdeTexto(const string &texto);

struct Simbolo
{
    string nombre;
    TipoDato tipo;
    int linea; // linea donde se declaro
};

class TablaSimbolos
{
private:
    vector<Simbolo> simbolos;

public:
    // Devuelve false si el nombre ya existia (declaracion duplicada)
    bool insertar(const string &nombre, TipoDato tipo, int linea);

    // Devuelve nullptr si el nombre no esta declarado
    const Simbolo *buscar(const string &nombre) const;

    const vector<Simbolo> &obtener() const;
};

#endif
