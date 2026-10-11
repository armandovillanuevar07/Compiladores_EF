#include "Postfija.h"

bool esOperando(const Token &t)
{
    return t.tipo == TipoToken::LITERAL_ENTERO ||
           t.tipo == TipoToken::LITERAL_REAL ||
           t.tipo == TipoToken::LITERAL_STRING ||
           t.tipo == TipoToken::IDENTIFICADOR;
}

bool esOperador(const Token &t)
{
    return t.tipo == TipoToken::OPERADOR;
}

// * y / se evaluan antes que + y -
int precedencia(const string &op)
{
    if (op == "*" || op == "/")
    {
        return 2;
    }
    if (op == "+" || op == "-")
    {
        return 1;
    }
    return 0;
}

vector<Token> aPostfija(const vector<Token> &infija)
{
    vector<Token> salida;
    vector<Token> pila; // operadores y parentesis

    for (const Token &t : infija)
    {
        if (esOperando(t))
        {
            // los operandos pasan directo a la salida
            salida.push_back(t);
        }
        else if (esOperador(t))
        {
            // sacamos los operadores de mayor o igual precedencia (asociatividad izquierda)
            while (!pila.empty() &&
                   esOperador(pila.back()) &&
                   precedencia(pila.back().lexema) >= precedencia(t.lexema))
            {
                salida.push_back(pila.back());
                pila.pop_back();
            }
            pila.push_back(t);
        }
        else if (t.tipo == TipoToken::DELIMITADOR && t.lexema == "(")
        {
            pila.push_back(t);
        }
        else if (t.tipo == TipoToken::DELIMITADOR && t.lexema == ")")
        {
            // vaciamos hasta encontrar el '(' que abre el grupo
            while (!pila.empty() && pila.back().lexema != "(")
            {
                salida.push_back(pila.back());
                pila.pop_back();
            }
            if (!pila.empty())
            {
                pila.pop_back(); // descartamos el '('
            }
        }
    }

    while (!pila.empty())
    {
        salida.push_back(pila.back());
        pila.pop_back();
    }

    return salida;
}

string tokensATexto(const vector<Token> &tokens)
{
    string texto = "";
    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (i > 0)
        {
            texto += " ";
        }
        texto += tokens[i].lexema;
    }
    return texto;
}
