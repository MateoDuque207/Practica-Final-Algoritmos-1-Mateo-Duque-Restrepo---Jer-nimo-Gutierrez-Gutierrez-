// ====================================================================
//  generador.h
// --------------------------------------------------------------------
//  Genera los conjuntos de datos para el benchmark.
//
//  Variamos:
//   - n : cantidad de numeros
//   - U : tamaño del universo (rango de las llaves)
//   - distribucion : como estan repartidos los numeros
//
//  Las semillas son fijas para que los resultados sean reproducibles
//  entre corridas.
// ====================================================================

#ifndef GENERADOR_H
#define GENERADOR_H

#include <vector>
#include <random>
#include <algorithm>
#include <string>
#include <cstdint>
#include <cmath>

namespace generador {

    using Llave = unsigned int;

    // Tipos de distribucion que vamos a probar
    enum class Distribucion {
        UNIFORME,        // numeros repartidos parejo en [0, U)
        CASI_ORDENADO,   // ya casi esta ordenado (pocos desordenes)
        DUPLICADOS,      // muchas llaves repetidas (poquitos valores)
        ASCENDENTE,      // ya esta ordenado de menor a mayor
        DESCENDENTE      // ya esta ordenado de mayor a menor
    };

    inline std::string nombre_distribucion(Distribucion d) {
        switch (d) {
            case Distribucion::UNIFORME:       return "uniforme";
            case Distribucion::CASI_ORDENADO:  return "casi-ordenado";
            case Distribucion::DUPLICADOS:     return "duplicados";
            case Distribucion::ASCENDENTE:     return "ascendente";
            case Distribucion::DESCENDENTE:    return "descendente";
        }
        return "?";
    }

    // ----------------------------------------------------------------
    //  generar()
    //  Devuelve un vector de tamaño n con llaves en [0, U) siguiendo
    //  la distribucion pedida. La semilla queda expuesta para poder
    //  hacer corridas reproducibles.
    // ----------------------------------------------------------------
    inline std::vector<Llave> generar(std::size_t n,
                                      std::size_t U,
                                      Distribucion d,
                                      std::uint64_t semilla = 7) {
        std::mt19937 rng(static_cast<std::uint32_t>(semilla));
        std::vector<Llave> v;
        v.reserve(n);

        if (d == Distribucion::UNIFORME) {
            std::uniform_int_distribution<Llave> uni(0, static_cast<Llave>(U - 1));
            for (std::size_t i = 0; i < n; ++i) v.push_back(uni(rng));
        }
        else if (d == Distribucion::CASI_ORDENADO) {
            // generamos uniforme, ordenamos y luego desordenamos un poquito
            std::uniform_int_distribution<Llave> uni(0, static_cast<Llave>(U - 1));
            for (std::size_t i = 0; i < n; ++i) v.push_back(uni(rng));
            std::sort(v.begin(), v.end());
            // hacemos n/100 swaps aleatorios (un 1% de desorden)
            std::size_t swaps = n / 100;
            std::uniform_int_distribution<std::size_t> idx(0, n - 1);
            for (std::size_t s = 0; s < swaps; ++s) {
                std::swap(v[idx(rng)], v[idx(rng)]);
            }
        }
        else if (d == Distribucion::DUPLICADOS) {
            // un universo chiquito a proposito (raiz cuadrada de U)
            // para que haya muchisimas repeticiones
            std::size_t U_pequeno = static_cast<std::size_t>(
                std::max<double>(2.0, std::sqrt((double)U)));
            std::uniform_int_distribution<Llave> uni(0,
                static_cast<Llave>(U_pequeno - 1));
            for (std::size_t i = 0; i < n; ++i) v.push_back(uni(rng));
        }
        else if (d == Distribucion::ASCENDENTE) {
            // ya viene ordenado de menor a mayor
            std::uniform_int_distribution<Llave> uni(0, static_cast<Llave>(U - 1));
            for (std::size_t i = 0; i < n; ++i) v.push_back(uni(rng));
            std::sort(v.begin(), v.end());
        }
        else { // DESCENDENTE
            std::uniform_int_distribution<Llave> uni(0, static_cast<Llave>(U - 1));
            for (std::size_t i = 0; i < n; ++i) v.push_back(uni(rng));
            std::sort(v.begin(), v.end(), std::greater<Llave>());
        }
        return v;
    }

} // namespace generador

#endif
