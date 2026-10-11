#!/bin/bash
# Ejecuta todos los casos de prueba de la carpeta pruebas/
# Uso (desde la carpeta EF):  ./ejecutar_pruebas.sh

if [ ! -f ./compilador ]; then
    echo "Primero compile el proyecto:  make   (o: g++ -std=c++17 *.cpp -o compilador)"
    exit 1
fi

for archivo in pruebas/*.txt; do
    echo
    echo "############################################################"
    echo "# CASO: $archivo"
    echo "############################################################"
    ./compilador "$archivo"
done
