# Informe Tecnico — Practica II

**Comparacion experimental: DialSort vs RadixSort vs std::sort**

Curso: ST0245-SI001 Estructuras de Datos y Algoritmos
Profesor: Alexander Narvaez Berrio
Universidad EAFIT — Escuela de Ciencias Aplicadas e Ingenieria
Integrantes: Mateo Duque Restrepo y Jeronimo Gutierrez Gutierrez
Abril 2026

---

## 1. Que hicimos

El objetivo era implementar DialSort y compararlo contra una alternativa escogida por nosotros, midiendo tiempo, memoria y throughput sobre conjuntos de datos enteros de tamaño variable.

Como alternativa principal escogimos **RadixSort** (base 256). La razon de esa eleccion es que tambien es un algoritmo de distribucion (no compara llaves, las distribuye), pero usa una idea distinta a la de DialSort: en vez de tener una cubeta por cada valor posible, hace varias pasadas mirando un byte a la vez. Esa diferencia hace que la comparacion sea mas interesante porque ambos son lineales pero por razones distintas.

Adicionalmente metimos `std::sort` de la STL como tercera estrategia, para tener un referente de algoritmo de comparacion bien optimizado (internamente es IntroSort: un hibrido de QuickSort + HeapSort + InsertionSort).

## 2. Como funciona cada algoritmo

### 2.1 DialSort

La idea es:
1. Reservar U cubetas vacias (una por cada valor posible de la llave).
2. Recorrer la entrada y meter cada numero en la cubeta que le corresponde por su valor: O(1) por insercion.
3. Recorrer las cubetas de 0 a U-1 y vaciarlas en orden hacia el arreglo de salida.

En nuestra implementacion las cubetas son `std::list<unsigned int>`. Lo hicimos asi porque es la forma "clasica" de bucket sort y permite ver bien que cada insercion es realmente O(1) sin ningun costo de redimensionamiento. **Spoiler para la seccion de resultados: esto tiene un costo en tiempo real porque cada nodo de `std::list` se asigna por separado en memoria, y eso es lento.** Lo discutimos en la seccion de conclusiones.

Complejidad teorica:

| Caso | Tiempo | Memoria |
|------|--------|---------|
| Mejor   | O(n + U) | O(n + U) |
| Promedio| O(n + U) | O(n + U) |
| Peor    | O(n + U) | O(n + U) |

DialSort no tiene un caso peor "sorpresa" en terminos de complejidad: depende solo de n y U, no del orden de los datos.

### 2.2 RadixSort

La idea es ordenar las llaves por bloques de bits, en este caso 1 byte (8 bits) a la vez. Para llaves de 32 bits, eso son 4 pasadas.

En cada pasada hacemos un Counting Sort estable usando ese byte como llave:
1. Contamos cuantos numeros tienen cada valor de byte (256 contadores).
2. Calculamos las posiciones acumuladas.
3. Recorremos la entrada y vamos colocando cada numero en su posicion correspondiente.

Empezamos por el byte menos significativo y vamos avanzando hacia el mas significativo. Como el Counting Sort es estable, el orden relativo se preserva entre pasadas, y al final el arreglo queda ordenado.

Complejidad teorica:

| Caso | Tiempo | Memoria |
|------|--------|---------|
| Mejor   | O(d·(n + b)) | O(n + b) |
| Promedio| O(d·(n + b)) | O(n + b) |
| Peor    | O(d·(n + b)) | O(n + b) |

donde d = 4 (bytes) y b = 256 (base). En la practica eso es esencialmente lineal en n.

### 2.3 std::sort (referencia)

`std::sort` de la STL es un IntroSort. Funciona asi:
- Empieza como QuickSort.
- Si la profundidad de recursion supera 2·log(n), cambia a HeapSort para evitar el caso peor cuadratico.
- Para tramos chiquitos (~16 elementos) usa InsertionSort que es mas rapido en ese rango.

Garantiza O(n log n) en cualquier caso y tiene un factor constante muy bueno porque es codigo super optimizado.

## 3. Diseño del benchmark

Para responder a la pregunta "cual algoritmo se desempeño mejor" necesitamos medir bajo distintas condiciones. Variamos:

**Tamaño de entrada n:**
100,000 / 500,000 / 1,000,000 / 5,000,000 / 10,000,000

(El requisito decia entre 100k y 10M, asi que cubrimos ese rango.)

**Tamaño del universo U:**
1024 y 65536. Con eso tenemos a DialSort en condiciones favorables (U chiquito) y desfavorables (U grande).

**Distribuciones:**
- `uniforme` — numeros repartidos parejo en [0, U)
- `casi-ordenado` — ordenado y luego un 1% de swaps aleatorios
- `duplicados` — universo reducido (sqrt(U)) para que haya muchas repeticiones
- `ascendente` — ya esta ordenado de menor a mayor
- `descendente` — ya esta ordenado de mayor a menor

**Repeticiones:**
Cada combinacion (algoritmo, n, U, distribucion) la corremos 5 veces con semillas distintas. Reportamos promedio y desviacion estandar.

**Mediciones:**
- Tiempo: `std::chrono::steady_clock`
- Memoria: lectura de VmRSS desde `/proc/self/status`
- Compilacion: `g++ -O2 -std=c++17`

Tambien metimos un `is_sorted_check` despues de cada corrida para asegurarnos de que ningun algoritmo este devolviendo basura.

## 4. Resultados

Los datos completos quedan en `resultados.csv` cuando se corre la opcion 2 del menu. Aqui mostramos lo mas representativo.

### 4.1 Variando n (uniforme, U = 1024)

| n         | DialSort | RadixSort | std::sort |
|-----------|----------|-----------|-----------|
| 100,000   | 0.0073 s | 0.0013 s  | 0.0043 s  |
| 500,000   | 0.0685 s | 0.0065 s  | 0.0219 s  |
| 1,000,000 | 0.1707 s | 0.0137 s  | 0.0431 s  |

Observaciones:
- **RadixSort es el mas rapido en esta categoria**, a veces 5x mas que `std::sort` y 12x mas que nuestro DialSort.
- DialSort escala lineal (como predice O(n+U)) pero con un factor constante grande por el uso de listas enlazadas.
- `std::sort` se mantiene predecible.

### 4.2 Efecto de la distribucion (n = 1,000,000, U = 1024)

| distribucion   | DialSort | RadixSort | std::sort |
|----------------|----------|-----------|-----------|
| uniforme       | 0.1707 s | 0.0137 s  | 0.0431 s  |
| casi-ordenado  | 0.0406 s | 0.0170 s  | 0.0093 s  |
| duplicados     | 0.1451 s | 0.0156 s  | 0.0271 s  |
| ascendente     | 0.0366 s | 0.0182 s  | 0.0078 s  |
| descendente    | 0.0311 s | 0.0170 s  | 0.0080 s  |

Observaciones:
- **`std::sort` se beneficia mucho de los datos casi ordenados** (gracias al InsertionSort que tiene como subrutina). En esa categoria gana.
- **RadixSort es indiferente a la distribucion**, como predice la teoria. Su tiempo se mantiene practicamente constante en 0.014–0.018 s.
- **DialSort tiene mas varianza**: rinde mejor en ascendente/descendente (los nodos de la lista quedan en mejor orden de cache) y peor en uniforme/duplicados (mas saltos de cache).
- En `duplicados`, DialSort no mejora aunque haya pocas cubetas activas, porque el costo de la lista enlazada con muchos nodos en una sola cubeta sigue siendo alto.

### 4.3 Efecto del universo U (n = 1,000,000, uniforme)

| U      | DialSort | RadixSort | std::sort |
|--------|----------|-----------|-----------|
| 1024   | 0.1707 s | 0.0137 s  | 0.0431 s  |
| 65,536 | 0.2013 s | 0.0133 s  | 0.0655 s  |

Observaciones:
- **DialSort empeora** un poco cuando U crece (porque tiene que recorrer mas cubetas vacias al final).
- **RadixSort es indiferente a U**: su costo depende del numero de bytes de la llave, no del rango.
- **`std::sort` empeora levemente** porque las llaves mas grandes producen menos repeticiones, lo cual rompe optimizaciones internas.

### 4.4 Memoria

| algoritmo  | RSS pico aprox (n=1M, U=65k) | Espacio teorico    |
|------------|-------------------------------|---------------------|
| DialSort   | ~40 MB                        | O(n + U)            |
| RadixSort  | ~20 MB                        | O(n + 256)          |
| std::sort  | ~20 MB                        | O(log n) sobre el arreglo |

DialSort gasta mas memoria por dos razones: cada nodo de `std::list` tiene overhead (next, prev, valor) y el arreglo de cubetas mismo es de tamaño U.

## 5. Analisis comparativo

| Criterio                      | DialSort         | RadixSort         | std::sort         |
|-------------------------------|------------------|-------------------|-------------------|
| Complejidad temporal          | O(n + U)         | O(d·(n+b))        | O(n log n)        |
| Complejidad espacial          | O(n + U)         | O(n + b)          | O(log n)          |
| Sensibilidad a distribucion   | Baja             | Ninguna           | Media             |
| Sensibilidad a U              | Alta             | Ninguna           | Baja              |
| Restriccion de las llaves     | Enteras y acotadas | Enteras de tamaño fijo | Cualquier comparable |
| Mejor escenario               | n grande, U muy chiquito | Casi cualquiera | Datos casi ordenados, datos genericos |

### ¿Cual algoritmo se desempeño mejor?

Depende de que estemos midiendo:

- **En tiempo bruto sobre datos uniformes: RadixSort gana**. Su comportamiento es muy predecible y lineal, y aprovecha bien el cache.
- **Sobre datos casi ordenados: `std::sort` gana** porque su InsertionSort interno aprovecha el orden parcial.
- **DialSort no fue el mas rapido en ninguna categoria**, pero es el que tiene la idea conceptual mas simple y es lineal en n+U siempre. La razon principal por la que se quedo atras en nuestras mediciones es la implementacion con `std::list` (ver seccion siguiente).

## 6. Conclusiones

1. **La estructura de datos importa tanto como el algoritmo.** Nuestro DialSort sobre `std::list` mostro mucho overhead por la asignacion individual de nodos. Una version con `std::vector<std::vector<Llave>>` muy probablemente seria varias veces mas rapida. Esto es una leccion fuerte: la complejidad teorica O(n+U) se cumplio (escala lineal), pero el factor constante es tan grande que se pierde en la practica.

2. **RadixSort es excelente para llaves enteras de tamaño fijo.** Su tiempo es estable, lineal, y no le importa la distribucion. Es la mejor opcion si las llaves son enteros y el tamaño es razonable.

3. **`std::sort` sigue siendo la opcion mas robusta para casos generales.** Nunca fue el mas lento por mucho margen, y gana en escenarios casi-ordenados que son muy comunes en la practica.

4. **La complejidad teorica predice la forma de la curva pero no la posicion absoluta.** Los tres algoritmos cumplieron sus complejidades teoricas, pero los factores constantes los reordenaron en la realidad. Hay que medir, no asumir.

5. **DialSort sigue siendo conceptualmente valioso** porque enseña la idea de ordenar sin comparar — es la base mental de RadixSort, BucketSort y CountingSort. Solo que en codigo de produccion casi nunca se usa la version con listas.

## 7. Anexos

- `main.cpp` — programa principal con el menu de tres modos
- `dialsort.h` — DialSort con cubetas como `std::list`
- `radixsort.h` — RadixSort base 256, 4 pasadas
- `stlsort.h` — wrapper sobre `std::sort`
- `generador.h` — genera datasets sinteticos con 5 distribuciones
- `medidor.h` — utilidades de cronometraje, estadistica y memoria
- `visual.h` — modo de simulacion paso a paso
- `resultados.csv` — generado al correr la opcion 2 del menu
