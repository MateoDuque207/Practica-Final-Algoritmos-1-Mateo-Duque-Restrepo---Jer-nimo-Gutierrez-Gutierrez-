// ====================================================================
//  main.cpp
// --------------------------------------------------------------------
//  Practica II - Estructuras de Datos y Algoritmos (ST0245-SI001)
//  EAFIT - Profesor Alexander Narvaez Berrio
//  Integrantes: Mateo Duque Restrepo
//               Jeronimo Gutierrez Gutierrez
//
//  Comparamos DialSort contra RadixSort y std::sort.
//
//  El programa tiene un menu chiquito donde uno escoge:
//   1) ver la simulacion paso a paso (entrada chiquita)
//   2) correr el benchmark completo (entradas grandes, varias veces)
//   3) correr una configuracion personalizada
//   0) salir
// ====================================================================

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstdio>

#include "dialsort.h"
#include "radixsort.h"
#include "stlsort.h"
#include "generador.h"
#include "medidor.h"
#include "visual.h"

using Llave = unsigned int;

// Cuantas veces repetimos cada configuracion para sacar promedio.
// 5 nos parecio suficiente: da una desviacion estable sin tomarse
// horas en los inputs grandes.
static const int VECES = 5;

// ----------------------------------------------------------------
//  esta_ordenado()
//  Verificacion al final de cada corrida. Si algo salio mal,
//  preferimos saberlo de una.
// ----------------------------------------------------------------
static bool esta_ordenado(const std::vector<Llave>& v) {
    for (std::size_t i = 1; i < v.size(); ++i) {
        if (v[i] < v[i - 1]) return false;
    }
    return true;
}

// ----------------------------------------------------------------
//  correr_una_config()
//  Corre el algoritmo VECES veces sobre datasets generados con
//  semillas distintas, calcula promedio, desviacion y throughput.
// ----------------------------------------------------------------
template <typename FuncOrden>
static medidor::Resultado correr_una_config(
        const std::string& nombre_alg,
        FuncOrden fn,
        std::size_t n, std::size_t U,
        generador::Distribucion d) {

    std::vector<double> tiempos;
    tiempos.reserve(VECES);
    std::size_t mem_pico = 0;

    for (int r = 0; r < VECES; ++r) {
        // dataset distinto cada vez (semillas 7, 8, 9, 10, 11)
        auto datos = generador::generar(n, U, d, 7 + r);

        // medimos
        double seg = medidor::cronometrar([&]() { fn(datos); });

        // verificamos
        if (!esta_ordenado(datos)) {
            std::cerr << "[!] " << nombre_alg
                      << " devolvio el arreglo desordenado!\n";
        }

        tiempos.push_back(seg);
        std::size_t m = medidor::leer_memoria_kb();
        if (m > mem_pico) mem_pico = m;
    }

    medidor::Resultado res;
    res.algoritmo         = nombre_alg;
    res.distribucion      = generador::nombre_distribucion(d);
    res.n                 = n;
    res.U                 = U;
    res.tiempo_promedio   = medidor::promedio(tiempos);
    res.tiempo_desviacion = medidor::desviacion(tiempos);
    res.throughput        = res.tiempo_promedio > 0.0
                            ? static_cast<double>(n) / res.tiempo_promedio
                            : 0.0;
    res.memoria_kb        = mem_pico;
    return res;
}

// ----------------------------------------------------------------
//  correr_benchmark_completo()
//  Recorre todas las combinaciones de (n, U, distribucion) que
//  queremos medir, corre los tres algoritmos en cada una y
//  guarda los resultados en resultados.csv.
// ----------------------------------------------------------------
static void correr_benchmark_completo() {
    std::vector<std::size_t> ns = {100000, 500000, 1000000, 5000000, 10000000};
    std::vector<std::size_t> Us = {1024, 65536};
    std::vector<generador::Distribucion> ds = {
        generador::Distribucion::UNIFORME,
        generador::Distribucion::CASI_ORDENADO,
        generador::Distribucion::DUPLICADOS,
        generador::Distribucion::ASCENDENTE,
        generador::Distribucion::DESCENDENTE
    };

    medidor::imprimir_encabezado();

    std::ofstream csv("resultados.csv");
    csv << "algoritmo,distribucion,n,U,promedio_seg,desviacion_seg,throughput,memoria_kb\n";

    for (std::size_t n : ns) {
        for (std::size_t U : Us) {
            for (generador::Distribucion d : ds) {

                // DialSort
                {
                    auto fn = [U](std::vector<Llave>& v) {
                        dialsort::ordenar(v, U);
                    };
                    auto r = correr_una_config("DialSort", fn, n, U, d);
                    medidor::imprimir_fila(r);
                    csv << r.algoritmo << "," << r.distribucion << ","
                        << r.n << "," << r.U << ","
                        << r.tiempo_promedio << "," << r.tiempo_desviacion << ","
                        << r.throughput << "," << r.memoria_kb << "\n";
                }
                // RadixSort
                {
                    auto fn = [](std::vector<Llave>& v) {
                        radixsort::ordenar(v);
                    };
                    auto r = correr_una_config("RadixSort", fn, n, U, d);
                    medidor::imprimir_fila(r);
                    csv << r.algoritmo << "," << r.distribucion << ","
                        << r.n << "," << r.U << ","
                        << r.tiempo_promedio << "," << r.tiempo_desviacion << ","
                        << r.throughput << "," << r.memoria_kb << "\n";
                }
                // std::sort
                {
                    auto fn = [](std::vector<Llave>& v) {
                        stlsort::ordenar(v);
                    };
                    auto r = correr_una_config("std::sort", fn, n, U, d);
                    medidor::imprimir_fila(r);
                    csv << r.algoritmo << "," << r.distribucion << ","
                        << r.n << "," << r.U << ","
                        << r.tiempo_promedio << "," << r.tiempo_desviacion << ","
                        << r.throughput << "," << r.memoria_kb << "\n";
                }
            }
        }
    }
    csv.close();
    std::cout << "\nresultados.csv generado.\n";
}

// ----------------------------------------------------------------
//  correr_simulacion()
//  Modo didactico: entrada chiquita, ASCII paso a paso.
// ----------------------------------------------------------------
static void correr_simulacion() {
    std::vector<Llave> chiquito = {6, 2, 8, 1, 9, 3, 5, 0, 7, 4,
                                   2, 6, 1, 8, 3, 9, 0, 5, 4, 7};
    visual::demo_dialsort(chiquito, 10);
    visual::demo_radixsort(chiquito);
}

// ----------------------------------------------------------------
//  correr_personalizado()
//  El usuario elige n, U y distribucion para una sola corrida.
// ----------------------------------------------------------------
static void correr_personalizado() {
    std::size_t n, U;
    int op;

    std::cout << "n (ej. 100000): ";
    std::cin >> n;
    std::cout << "U (ej. 1024)  : ";
    std::cin >> U;
    std::cout << "distribucion:\n"
              << "  0) uniforme\n"
              << "  1) casi-ordenado\n"
              << "  2) duplicados\n"
              << "  3) ascendente\n"
              << "  4) descendente\n"
              << "opcion: ";
    std::cin >> op;

    auto d = static_cast<generador::Distribucion>(op);

    medidor::imprimir_encabezado();

    {
        auto fn = [U](std::vector<Llave>& v) { dialsort::ordenar(v, U); };
        medidor::imprimir_fila(correr_una_config("DialSort",  fn, n, U, d));
    }
    {
        auto fn = [](std::vector<Llave>& v) { radixsort::ordenar(v); };
        medidor::imprimir_fila(correr_una_config("RadixSort", fn, n, U, d));
    }
    {
        auto fn = [](std::vector<Llave>& v) { stlsort::ordenar(v); };
        medidor::imprimir_fila(correr_una_config("std::sort", fn, n, U, d));
    }
}

// ----------------------------------------------------------------
//  main()
// ----------------------------------------------------------------
int main(int argc, char** argv) {
    // si el usuario pasa "bench" o "demo" como argumento, vamos
    // directo a esa opcion (util para correr desde la terminal)
    if (argc >= 2) {
        std::string modo = argv[1];
        if (modo == "bench") { correr_benchmark_completo(); return 0; }
        if (modo == "demo")  { correr_simulacion(); return 0; }
    }

    while (true) {
        std::cout << "\n=== Practica II - DialSort vs RadixSort vs std::sort ===\n"
                  << "  1) Simulacion paso a paso (entrada chiquita)\n"
                  << "  2) Benchmark completo (genera resultados.csv)\n"
                  << "  3) Corrida personalizada (n / U / distribucion)\n"
                  << "  0) Salir\n"
                  << "opcion: ";
        int op;
        if (!(std::cin >> op)) break;

        if      (op == 0) break;
        else if (op == 1) correr_simulacion();
        else if (op == 2) correr_benchmark_completo();
        else if (op == 3) correr_personalizado();
        else std::cout << "opcion invalida.\n";
    }
    return 0;
}
