# Mini compilador - Evaluacion Final de Compiladores

Mini compilador en C++ que integra las fases:

```
Codigo fuente -> Lexer -> Parser -> Analisis semantico -> Codigo intermedio -> Optimizacion
```

## Como compilar

Entre a la carpeta del proyecto (la que contiene `main.cpp`, `Lexer.h`, etc.) y ejecute **uno** de estos comandos:

```bash
make
```
o bien
```bash
g++ -std=c++17 *.cpp -o compilador
```

> Importante: el comando debe ejecutarse **dentro** de la carpeta donde estan los archivos `.h` y `.cpp`.
> Si se ejecuta desde otra carpeta aparece el error `'Lexer.h' file not found`.

## Como ejecutar

Con un archivo de prueba:
```bash
./compilador pruebas/caso8_codigo_intermedio.txt
```

Escribiendo el codigo por teclado (varias lineas; termine con una linea que diga `FIN` o con Ctrl+D):
```bash
./compilador
```

Para ejecutar todos los casos de prueba:
```bash
./ejecutar_pruebas.sh
```

## Gramatica

```
Programa    -> Sentencia*
Sentencia   -> Declaracion | Asignacion
Declaracion -> Tipo ID = Expresion ;
Asignacion  -> ID = Expresion ;
Tipo        -> int | float | string
Expresion   -> Termino { (+ | -) Termino }
Termino     -> Factor  { (* | /) Factor }
Factor      -> Valor | ( Expresion )
Valor       -> NUM | REAL | STRING | ID
```

Cuando la expresion es un solo valor se obtiene la regla minima de la consigna:
`Declaracion -> Tipo ID = Valor ;` con `Valor -> NUM | REAL | STRING | ID`.
`STRING` es un literal delimitado por comillas dobles: `"10.5"` es un string y `"1"` es un string.

## Fases

| Fase | Archivos | Que hace |
|------|----------|----------|
| Lexer | `Lexer.h/.cpp`, `Token.h` | Tokens (palabras reservadas, identificadores, enteros, reales, strings, operadores, `=`, delimitadores `; ( )`). Procesa varias lineas, guarda los errores en una lista y continua. |
| Parser | `Parser.h/.cpp`, `Sentencia.h` | Analisis descendente recursivo. Reconoce varias sentencias y se recupera de los errores (modo panico) para seguir analizando. |
| Semantico | `Semantico.h/.cpp`, `TablaSimbolos.h/.cpp` | Tabla de simbolos (nombre -> tipo), variable duplicada, identificador no declarado, compatibilidad de tipos. |
| Postfija | `Postfija.h/.cpp` | Conversion infija -> postfija (algoritmo de pila de operadores). |
| Codigo intermedio | `CodigoIntermedio.h/.cpp` | Codigo de tres direcciones con temporales `t1, t2, ...`. |
| Optimizacion | `CodigoIntermedio.cpp` | Plegado de constantes, propagacion de constantes en temporales y eliminacion de copias redundantes. |

### Reglas de compatibilidad de tipos

- `int <- int`, `float <- float`, `string <- string`
- `float <- int` se permite (ampliacion)
- `int <- float` es error (perdida de precision)
- `string` con `int`/`float` es error, y los strings no se pueden usar con `+ - * /`

## Casos de prueba (carpeta `pruebas/`)

| Archivo | Que demuestra |
|---------|---------------|
| `caso1_declaraciones_validas.txt` | Declaraciones validas de `int` y `float` |
| `caso2_string.txt` | `string nombre = "Ana";`, `"10.5"` y `"1"` como strings |
| `caso3_variable_duplicada.txt` | Variable declarada dos veces |
| `caso4_no_declarado.txt` | Uso de identificadores no declarados |
| `caso5_error_sintactico.txt` | Instruccion con error sintactico (el analisis continua) |
| `caso6_error_lexico_continua.txt` | Error lexico seguido de una instruccion valida |
| `caso7_varias_lineas.txt` | Codigo de varias lineas |
| `caso8_codigo_intermedio.txt` | `resultado = (a + b) * c;` -> codigo de tres direcciones |
| `caso9_optimizacion.txt` | Antes/despues de la optimizacion (`t1 = 2 + 3` -> `resultado = 5`) |
| `caso10_tipos_incompatibles.txt` | Errores de compatibilidad de tipos |
| `caso11_string_sin_cerrar.txt` | String sin comilla de cierre |

## Limitaciones conocidas

- No hay numeros negativos literales (`-5`) ni operador unario.
- Los strings no admiten secuencias de escape ni saltos de linea.
- `if` y `else` se reconocen como palabras reservadas, pero la gramatica aun no las usa.
- El codigo intermedio se genera solo para las sentencias sin errores.
