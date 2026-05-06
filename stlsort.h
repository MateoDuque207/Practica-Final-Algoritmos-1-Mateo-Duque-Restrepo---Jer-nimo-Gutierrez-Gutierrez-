// ====================================================================
//  stlsort.h
// --------------------------------------------------------------------
//  Tercera estrategia: el std::sort de la libreria estandar.
//
//  Lo metimos como referencia industrial. Internamente std::sort es
//  un IntroSort (mezcla de QuickSort, HeapSort e InsertionSort) y
//  garantiza O(n log n) en cualquier caso. Es lo que usaria cualquier
//  programador en codigo real.
//
//  La tenemos para comparar contra que tan bien quedaron nuestras
//  implementaciones a mano.
// ====================================================================

#ifndef STLSORT_H
#define STLSORT_H

#include <vector>
#include <algorithm>

namespace stlsort {

    using Llave = unsigned int;

    inline void ordenar(std::vector<Llave>& entrada) {
        std::sort(entrada.begin(), entrada.end());
    }

} // namespace stlsort

#endif
