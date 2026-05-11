// ====================================================================
//  dialsort.h
// --------------------------------------------------------------------
//  DialSort: ordenamiento por distribucion en cubetas.
//
//  La idea es: como sabemos que las llaves estan en [0, U),
//  reservamos U "cubetas" (listas vacias). Recorremos la entrada y a
//  cada numero lo metemos en la cubeta que le corresponde por su valor.
//  Al final, leemos las cubetas en orden de 0 a U-1 y vamos escribiendo
//  los numeros de vuelta al arreglo. Listo, queda ordenado.
//
//  Por que sirve: no comparamos llaves, solo indexamos. Eso lo vuelve
//  lineal en n + U.
//
//  Por que NO siempre sirve: si U es gigante y n chico, gastamos un
//  monton de memoria en cubetas vacias y el barrido final es lento.
// ====================================================================

#ifndef DIALSORT_H
#define DIALSORT_H

#include <vector>
#include <list>
#include <cstddef>

namespace dialsort {

    // Tipo de la llave que vamos a ordenar
    using Llave = unsigned int;

    // ----------------------------------------------------------------
    //  ordenar()
    //  - entrada: el arreglo de numeros (se ordena en sitio)
    //  - U      : tamaño del universo, las llaves viven en [0, U)
    // ----------------------------------------------------------------
    inline void ordenar(std::vector<Llave>& entrada, std::size_t U) {

        // Creamos U cubetas vacias. Usamos std::list (lista enlazada)
        // porque inserta en O(1) sin tener que mover nada en memoria.
        // Es la forma "clasica" de ensenar bucket sort.
        std::vector<std::list<Llave>> cubetas(U);

        // Fase 1: recorrer la entrada y meter cada numero en su cubeta.
        // Costo: O(n) inserciones constantes.
        for (std::size_t i = 0; i < entrada.size(); ++i) {
            Llave x = entrada[i];
            cubetas[x].push_back(x);
        }

        // Fase 2: leer las cubetas en orden y reescribir la entrada.
        // Costo: O(U + n)  porque pasamos por todas las cubetas
        // (incluso las vacias) y al final escribimos n elementos.
        std::size_t pos = 0;
        for (std::size_t k = 0; k < U; ++k) {
            for (Llave v : cubetas[k]) {
                entrada[pos] = v;
                ++pos;
            }
        }
        // Ya quedo ordenada la entrada.
    }

    // ----------------------------------------------------------------
    //  ordenar_con_traza()
    //  Misma idea pero imprimiendo paso a paso lo que esta pasando.
    //  Solo para entradas pequeñas, es la version "didactica".
    // ----------------------------------------------------------------
    template <typename Stream>
    inline void ordenar_con_traza(std::vector<Llave>& entrada,
                                  std::size_t U, Stream& out) {

        std::vector<std::list<Llave>> cubetas(U);

        out << "[DialSort] metiendo numeros en sus cubetas...\n";
        for (std::size_t i = 0; i < entrada.size(); ++i) {
            Llave x = entrada[i];
            cubetas[x].push_back(x);
            out << "  paso " << (i + 1) << ": el " << x
                << " se va a cubeta[" << x << "]\n";
        }

        out << "[DialSort] estado de las cubetas:\n";
        for (std::size_t k = 0; k < U; ++k) {
            out << "  cubeta[" << k << "] -> ";
            for (Llave v : cubetas[k]) out << v << " ";
            out << "\n";
        }

        out << "[DialSort] vaciando cubetas en orden...\n";
        std::size_t pos = 0;
        for (std::size_t k = 0; k < U; ++k) {
            for (Llave v : cubetas[k]) {
                entrada[pos++] = v;
            }
        }
        out << "[DialSort] listo.\n";
    }

} // namespace dialsort

#endif
