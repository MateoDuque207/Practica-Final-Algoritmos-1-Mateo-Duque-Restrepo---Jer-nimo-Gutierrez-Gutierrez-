// ====================================================================
//  pasos.h
// --------------------------------------------------------------------
//  Versiones de los algoritmos que van guardando un "snapshot" del
//  arreglo despues de cada operacion. La GUI usa estos snapshots
//  para reproducir la animacion.
//
//  Solo se debe usar con entradas chiquitas (n <= 200) porque cada
//  snapshot copia todo el arreglo y para n grande explota la memoria.
// ====================================================================

#ifndef PASOS_H
#define PASOS_H

#include <vector>
#include <list>
#include <string>
#include <cstddef>
#include <cstdint>

namespace pasos {

    using Llave = unsigned int;

    // ----------------------------------------------------------------
    //  Snapshot: una "foto" del arreglo en un momento del proceso.
    //  Guardamos el arreglo, el indice resaltado (la posicion sobre la
    //  que se esta operando) y un mensaje corto que la GUI puede
    //  mostrar abajo para que el espectador entienda que esta pasando.
    // ----------------------------------------------------------------
    struct Snapshot {
        std::vector<Llave> arr;
        int resaltado = -1;     // -1 = ningun indice resaltado
        std::string mensaje;
    };

    // ----------------------------------------------------------------
    //  dialsort_pasos()
    //  Ejecuta DialSort guardando un snapshot por cada operacion.
    // ----------------------------------------------------------------
    inline std::vector<Snapshot> dialsort_pasos(std::vector<Llave> entrada,
                                                std::size_t U) {
        std::vector<Snapshot> resultado;
        resultado.push_back({entrada, -1, "Estado inicial"});

        std::vector<std::list<Llave>> cubetas(U);

        // Fase 1: meter cada numero en su cubeta
        for (std::size_t i = 0; i < entrada.size(); ++i) {
            Llave x = entrada[i];
            cubetas[x].push_back(x);
            Snapshot s;
            s.arr = entrada;
            s.resaltado = static_cast<int>(i);
            s.mensaje = "Insertando " + std::to_string(x) +
                        " -> cubeta[" + std::to_string(x) + "]";
            resultado.push_back(s);
        }

        // Fase 2: vaciar las cubetas en orden
        std::size_t pos = 0;
        for (std::size_t k = 0; k < U; ++k) {
            for (Llave v : cubetas[k]) {
                entrada[pos] = v;
                Snapshot s;
                s.arr = entrada;
                s.resaltado = static_cast<int>(pos);
                s.mensaje = "Vaciando cubeta[" + std::to_string(k) +
                            "] -> posicion " + std::to_string(pos);
                resultado.push_back(s);
                ++pos;
            }
        }

        // Snapshot final
        resultado.push_back({entrada, -1, "Listo, arreglo ordenado"});
        return resultado;
    }

    // ----------------------------------------------------------------
    //  radixsort_pasos()
    //  Ejecuta RadixSort y guarda un snapshot al final de cada pasada
    //  de byte. Adicionalmente en cada pasada va guardando snapshots
    //  intermedios para que se vea la animacion.
    // ----------------------------------------------------------------
    inline std::vector<Snapshot> radixsort_pasos(std::vector<Llave> entrada) {
        std::vector<Snapshot> resultado;
        resultado.push_back({entrada, -1, "Estado inicial"});
        if (entrada.empty()) return resultado;

        std::vector<Llave> buffer(entrada.size());

        for (int byte = 0; byte < 4; ++byte) {

            // determinamos cual vector es la fuente y cual el destino
            const std::vector<Llave>& src =
                (byte % 2 == 0) ? entrada : buffer;
            std::vector<Llave>& dst =
                (byte % 2 == 0) ? buffer : entrada;

            // counting sort estable usando el byte indicado
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

            // distribuimos los numeros en el destino, guardando snapshot
            // cada cierto numero de inserciones para que se vea fluido
            std::size_t cada = std::max<std::size_t>(1, src.size() / 30);
            for (std::size_t i = 0; i < src.size(); ++i) {
                unsigned int b = (src[i] >> (byte * 8)) & 0xFF;
                dst[conteo[b]] = src[i];
                conteo[b]++;
                if (i % cada == 0) {
                    Snapshot s;
                    s.arr = dst;
                    s.resaltado = static_cast<int>(conteo[b] - 1);
                    s.mensaje = "Pasada byte " + std::to_string(byte) +
                                ", colocando " + std::to_string(src[i]);
                    resultado.push_back(s);
                }
            }

            // snapshot al terminar la pasada
            Snapshot s;
            s.arr = dst;
            s.resaltado = -1;
            s.mensaje = "Termino pasada del byte " + std::to_string(byte);
            resultado.push_back(s);
        }

        // como hicimos 4 pasadas (par), el resultado quedo en "entrada"
        resultado.push_back({entrada, -1, "Listo, arreglo ordenado"});
        return resultado;
    }

} // namespace pasos

#endif
