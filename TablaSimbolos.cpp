#include "TablaSimbolos.h"

string nombreTipo(TipoDato tipo)
{
    switch (tipo)
    {
    case TipoDato::ENTERO:
        return "int";
    case TipoDato::REAL:
        return "float";
    case TipoDato::CADENA:
        return "string";
    case TipoDato::INVALIDO:
        return "invalido";
    }
    return "invalido";
}

TipoDato tipoDesdeTexto(const string &texto)
{
    if (texto == "int")
    {
        return TipoDato::ENTERO;
    }
    if (texto == "float")
    {
        return TipoDato::REAL;
    }
    if (texto == "string")
    {
        return TipoDato::CADENA;
    }
    return TipoDato::INVALIDO;
}

bool TablaSimbolos::insertar(const string &nombre, TipoDato tipo, int linea)
{
    if (buscar(nombre) != nullptr)
    {
        return false;
    }

    simbolos.push_back({nombre, tipo, linea});
    return true;
}

const Simbolo *TablaSimbolos::buscar(const string &nombre) const
{
    for (const Simbolo &s : simbolos)
    {
        if (s.nombre == nombre)
        {
            return &s;
        }
    }
    return nullptr;
}

const vector<Simbolo> &TablaSimbolos::obtener() const
{
    return simbolos;
}
