CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
FUENTES  = main.cpp Lexer.cpp Parser.cpp TablaSimbolos.cpp Semantico.cpp Postfija.cpp CodigoIntermedio.cpp

compilador: $(FUENTES) $(wildcard *.h)
	$(CXX) $(CXXFLAGS) $(FUENTES) -o compilador

limpiar:
	rm -f compilador
