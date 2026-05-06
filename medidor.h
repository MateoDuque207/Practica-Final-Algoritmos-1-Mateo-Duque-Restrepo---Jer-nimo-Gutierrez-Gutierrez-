// ====================================================================
//  medidor.h
// --------------------------------------------------------------------
//  Utilidades para medir:
//   - tiempo de ejecucion
//   - media y desviacion estandar
//   - throughput (numeros ordenados por segundo)
//   - memoria residente del proceso (en Linux)
//
//  Cada algoritmo lo corremos varias veces por configuracion para
//  no quedarnos con un solo numero, sino con un promedio.
// ====================================================================

#ifndef MEDIDOR_H
#define MEDIDOR_H

#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace medidor {

    // ----------------------------------------------------------------
    //  cronometrar()
    //  Corre la funcion y devuelve cuantos segundos se demoro.
    //  Usamos steady_clock para que no nos afecten cambios del reloj
    //  del sistema durante la corrida.
    // ----------------------------------------------------------------
    template <typename F>
    inline double cronometrar(F&& f) {
        auto inicio = std::chrono::steady_clock::now();
        f();
        auto fin = std::chrono::steady_clock::now();
        std::chrono::duration<double> dt = fin - inicio;
        return dt.count();
    }

    // ----------------------------------------------------------------
    //  promedio()
    //  Media aritmetica simple
    // ----------------------------------------------------------------
    inline double promedio(const std::vector<double>& xs) {
        if (xs.empty()) return 0.0;
        double suma = 0.0;
        for (double x : xs) suma += x;
        return suma / xs.size();
    }

    // ----------------------------------------------------------------
    //  desviacion()
    //  Desviacion estandar muestral (con n-1 en el denominador).
    //  Si solo hay un dato devolvemos 0 para no dividir por cero.
    // ----------------------------------------------------------------
    inline double desviacion(const std::vector<double>& xs) {
        if (xs.size() < 2) return 0.0;
        double m = promedio(xs);
        double acum = 0.0;
        for (double x : xs) {
            double d = x - m;
            acum += d * d;
        }
        return std::sqrt(acum / (xs.size() - 1));
    }

    // ----------------------------------------------------------------
    //  leer_memoria_kb()
    //  Lee la memoria residente del proceso desde /proc/self/status.
    //  Si no estamos en Linux, devuelve 0 (no es un error grave).
    // ----------------------------------------------------------------
    inline std::size_t leer_memoria_kb() {
        std::ifstream f("/proc/self/status");
        if (!f.is_open()) return 0;
        std::string linea;
        while (std::getline(f, linea)) {
            // buscamos la linea que empieza por "VmRSS:"
            if (linea.rfind("VmRSS:", 0) == 0) {
                std::size_t kb = 0;
                const char* p = linea.c_str() + 6;
                // saltamos espacios
                while (*p == ' ' || *p == '\t') ++p;
                // leemos los digitos
                while (*p >= '0' && *p <= '9') {
                    kb = kb * 10 + (*p - '0');
                    ++p;
                }
                return kb;
            }
        }
        return 0;
    }

    // ----------------------------------------------------------------
    //  Resultado: bundle de numeros que reportamos por configuracion
    // ----------------------------------------------------------------
    struct Resultado {
        std::string algoritmo;
        std::string distribucion;
        std::size_t n;
        std::size_t U;
        double      tiempo_promedio;
        double      tiempo_desviacion;
        double      throughput;
        std::size_t memoria_kb;
    };

    // imprime una linea de resultado en formato de tabla
    inline void imprimir_fila(const Resultado& r) {
        std::printf(" %-10s | %-13s | %10zu | %10zu | %9.4f s | %8.4f s | %12.2e n/s | %7zu KB\n",
                    r.algoritmo.c_str(), r.distribucion.c_str(),
                    r.n, r.U,
                    r.tiempo_promedio, r.tiempo_desviacion,
                    r.throughput,
                    r.memoria_kb);
    }

    // imprime el encabezado de la tabla
    inline void imprimir_encabezado() {
        std::printf("\n");
        std::printf(" %-10s | %-13s | %10s | %10s | %11s | %10s | %16s | %10s\n",
                    "algoritmo", "distribucion", "n", "U",
                    "promedio", "desv.std", "throughput", "memoria");
        std::printf(" -----------+---------------+------------+------------+-------------+------------+------------------+-----------\n");
    }

} // namespace medidor

#endif
