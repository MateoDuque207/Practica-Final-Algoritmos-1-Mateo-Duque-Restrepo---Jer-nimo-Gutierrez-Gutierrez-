# Practica II — DialSort vs RadixSort vs std::sort

**Curso:** Estructuras de Datos y Algoritmos
**Universidad:** EAFIT — Escuela de Ciencias Aplicadas e Ingenieria
**Profesor:** Alexander Narvaez Berrio
**Integrantes:** Mateo Duque Restrepo, Jeronimo Gutierrez Gutierrez

## ¿De que va esto?

Esta es la Practica II del curso. Implementamos **DialSort** y lo comparamos contra una alternativa que escogimos nosotros: **RadixSort** (base 256). Tambien metimos `std::sort` como tercer algoritmo para tener una referencia industrial.

El proyecto tiene dos ejecutables:
- `practica2` — version de consola con el menu, el benchmark y la simulacion paso a paso.
- `practica2_gui` — interfaz grafica con SFML para visualizar como se va ordenando el arreglo en tiempo real.

## Estructura del proyecto

```
.
├── main.cpp          -> programa de consola (menu + benchmark)
├── gui.cpp           -> programa con interfaz grafica SFML
├── dialsort.h        -> DialSort con cubetas implementadas como std::list
├── radixsort.h       -> RadixSort base 256 (4 pasadas para uint32)
├── stlsort.h         -> wrapper de std::sort
├── generador.h       -> genera datasets con distintas distribuciones
├── medidor.h         -> cronometro, promedio, desviacion, memoria
├── visual.h          -> simulacion paso a paso para entradas chiquitas (consola)
├── pasos.h           -> versiones que graban snapshots para la GUI
├── CMakeLists.txt
├── resultados.csv    -> se genera al correr el benchmark
└── README.md         -> este archivo
```

## Instalacion de SFML

La GUI necesita SFML 2.5 o superior. Si no la instalas, igual podes correr la version de consola.

### Windows con CLion + vcpkg (recomendado)

1. Instalar vcpkg desde https://github.com/microsoft/vcpkg
2. En la consola: `vcpkg install sfml`
3. En CLion: Settings → Build → CMake → CMake options:
   `-DCMAKE_TOOLCHAIN_FILE=C:/ruta/a/vcpkg/scripts/buildsystems/vcpkg.cmake`

### Windows manual

1. Descargar SFML 2.6 desde https://www.sfml-dev.org/download.php
2. Extraer en `C:/SFML`
3. En CLion: Settings → Build → CMake → CMake options:
   `-DSFML_DIR=C:/SFML/lib/cmake/SFML`

### Ubuntu / Debian

```
sudo apt install libsfml-dev
```

### macOS

```
brew install sfml
```

## Como correrlo

### Opcion 1 — Desde CLion

1. Abrir la carpeta del proyecto.
2. CLion detecta el `CMakeLists.txt` automaticamente.
3. Si SFML esta instalado, vas a tener dos targets: `practica2` (consola) y `practica2_gui` (interfaz).
4. Si SFML no esta, solo aparece `practica2`.
5. Click en Run.

### Opcion 2 — Desde la terminal

Consola:
```
g++ -std=c++17 -O2 main.cpp -o practica2
./practica2
```

Interfaz grafica:
```
g++ -std=c++17 -O2 gui.cpp -lsfml-graphics -lsfml-window -lsfml-system -o practica2_gui
./practica2_gui
```

## Que hace la version de consola

Aparece un menu:
- `1` muestra la simulacion paso a paso (entrada chiquita) en ASCII.
- `2` corre el benchmark completo y genera `resultados.csv`.
- `3` deja escoger n, U y distribucion para una corrida personalizada.
- `0` sale.

## Que hace la version grafica

Una ventana sencilla con:
- Botones para escoger algoritmo (DialSort, RadixSort, std::sort).
- Botones para escoger distribucion (uniforme, casi-ordenado, duplicados, ascendente, descendente).
- Botones para velocidad de animacion (lento, normal, rapido).
- Boton "Generar nuevo" que crea un nuevo dataset aleatorio.
- Boton "Ordenar" que arranca la animacion.

El area central muestra el arreglo como barras de colores y se va animando segun avanza el algoritmo. Abajo aparece el tiempo real medido y el paso actual.

**Nota sobre la fuente:** la GUI intenta cargar fuentes comunes del sistema automaticamente (Arial en Windows, DejaVu en Linux). Si no encuentra ninguna, las barras igual se muestran pero sin texto en los botones. Si pasa eso, copia un archivo `.ttf` (por ejemplo `arial.ttf`) en la misma carpeta del ejecutable.

## Que mide el benchmark

Variamos:
- `n` ∈ {100k, 500k, 1M, 5M, 10M}
- `U` ∈ {1024, 65536}
- distribucion ∈ {uniforme, casi-ordenado, duplicados, ascendente, descendente}

Cada combinacion la corremos 5 veces con semillas distintas y reportamos:
- tiempo promedio
- desviacion estandar
- throughput (numeros ordenados por segundo)
- pico de memoria residente (RSS)

## Resumen de complejidad

| Algoritmo  | Tiempo                | Memoria   | Mejor cuando...                       |
|------------|-----------------------|-----------|----------------------------------------|
| DialSort   | O(n + U)              | O(n + U)  | U es chiquito comparado con n          |
| RadixSort  | O(d·(n + b))          | O(n + b)  | siempre lineal, no depende de la dist. |
| std::sort  | O(n log n) garantizado| O(log n)  | uso general, robusto siempre           |

(d = numero de bytes de la llave = 4 para uint32; b = base = 256)

## Resultados destacados (n = 1,000,000, U = 1024)

| algoritmo  | distribucion  | promedio  | throughput     |
|------------|---------------|-----------|----------------|
| DialSort   | uniforme      | 0.171 s   | 5.86e+06 n/s   |
| RadixSort  | uniforme      | 0.014 s   | 7.32e+07 n/s   |
| std::sort  | uniforme      | 0.043 s   | 2.32e+07 n/s   |
| DialSort   | duplicados    | 0.145 s   | 6.89e+06 n/s   |
| RadixSort  | duplicados    | 0.016 s   | 6.39e+07 n/s   |
| std::sort  | duplicados    | 0.027 s   | 3.69e+07 n/s   |

(ver `resultados.csv` despues de correr el benchmark para la tabla completa)
