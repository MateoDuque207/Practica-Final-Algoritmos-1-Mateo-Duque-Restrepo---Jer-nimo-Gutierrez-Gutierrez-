// ====================================================================
//  visual.h
// --------------------------------------------------------------------
//  Visualizacion del comportamiento interno de los algoritmos.
//  Pintamos en consola que esta pasando paso a paso, con una entrada
//  pequeña para que se entienda. Si lo hicieramos con n grande la
//  consola se llena y no se ve nada.
// ====================================================================

#ifndef VISUAL_H
#define VISUAL_H

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <cstdio>

namespace visual {

    using Llave = unsigned int;

    // helper para imprimir un arreglo con etiqueta
    inline void mostrar(const std::vector<Llave>& v,
                        const std::string& etiqueta) {
        std::cout << etiqueta << " [ ";
        for (Llave x : v) std::cout << x << " ";
        std::cout << "]\n";
    }

    // ----------------------------------------------------------------
    //  demo_dialsort()
    //  Muestra como van llenandose las cubetas paso a paso y luego
    //  como se vacian en orden.
    // ----------------------------------------------------------------
    inline void demo_dialsort(std::vector<Llave> entrada, std::size_t U) {
        std::cout << "\n+++++++++ DEMO DIALSORT +++++++++\n";
        mostrar(entrada, "entrada");

        std::vector<std::list<Llave>> cubetas(U);

        std::cout << "\n>> fase 1: meter cada numero en su cubeta\n";
        for (std::size_t i = 0; i < entrada.size(); ++i) {
            Llave x = entrada[i];
            cubetas[x].push_back(x);
            std::cout << "   - el " << x << " se va a cubeta[" << x << "]\n";
        }

        std::cout << "\n>> fase 2: como quedaron las cubetas\n";
        for (std::size_t k = 0; k < U; ++k) {
            std::printf("   cubeta[%2zu] : ", k);
            for (Llave v : cubetas[k]) std::cout << v << " ";
            std::cout << "\n";
        }

        std::cout << "\n>> fase 3: vaciar cubetas en orden\n";
        std::vector<Llave> salida;
        salida.reserve(entrada.size());
        for (std::size_t k = 0; k < U; ++k)
            for (Llave v : cubetas[k]) salida.push_back(v);

        mostrar(salida, "ordenado");
        std::cout << "+++++++++++++++++++++++++++++++++\n";
    }

    // ----------------------------------------------------------------
    //  demo_radixsort()
    //  Muestra como queda el arreglo despues de cada pasada de byte.
    //  Como nuestros numeros de prueba son pequeños, solo el byte 0
    //  va a tener informacion. Las pasadas siguientes no cambian nada
    //  pero igual las imprimimos para que se vea el flujo completo.
    // ----------------------------------------------------------------
    inline void demo_radixsort(std::vector<Llave> entrada) {
        std::cout << "\n+++++++++ DEMO RADIXSORT +++++++++\n";
        mostrar(entrada, "entrada");
        std::cout << "\nradix base 256, hace 4 pasadas (una por byte):\n";

        std::vector<Llave> buffer(entrada.size());

        for (int byte = 0; byte < 4; ++byte) {

            // counting sort estable usando ese byte como llave
            const std::vector<Llave>& src =
                (byte % 2 == 0) ? entrada : buffer;
            std::vector<Llave>& dst =
                (byte % 2 == 0) ? buffer : entrada;

            std::vector<std::size_t> conteo(256, 0);
            for (std::size_t i = 0; i < src.size(); ++i) {
                unsigned int b = (src[i] >> (byte * 8)) & 0xFF;
                conteo[b]++;
            }
            std::size_t acum = 0;
            for (int j = 0; j < 256; ++j) {
                std::size_t aux = conteo[j];
                conteo[j] = acum;
                acum += aux;
            }
            for (std::size_t i = 0; i < src.size(); ++i) {
                unsigned int b = (src[i] >> (byte * 8)) & 0xFF;
                dst[conteo[b]] = src[i];
                conteo[b]++;
            }

            std::cout << "   pasada byte " << byte << ": ";
            for (Llave v : dst) std::cout << v << " ";
            std::cout << "\n";
        }

        // tras 4 pasadas, los datos quedaron en "entrada"
        // (porque hicimos un numero par de intercambios src/dst)
        mostrar(entrada, "ordenado");
        std::cout << "++++++++++++++++++++++++++++++++++\n";
    }

} // namespace visual

#endif
