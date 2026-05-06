// ====================================================================
//  radixsort.h
// --------------------------------------------------------------------
//  RadixSort: ordena numeros enteros mirando un "digito" a la vez.
//
//  La idea: no comparamos llaves enteras, las ordenamos por bloques
//  de bits. Aca usamos base 256 (un byte) porque es muy rapida de
//  manipular. Para llaves de 32 bits, hacemos 4 pasadas (una por byte).
//
//  En cada pasada hacemos un counting sort estable usando ese byte
//  como llave. Despues de las 4 pasadas, el arreglo queda ordenado.
//
//  Por que la escogimos como alternativa a DialSort:
//   - Es otro algoritmo de distribucion (no compara llaves), entonces
//     la comparacion es interesante: ambos son lineales pero por
//     razones distintas.
//   - DialSort sufre cuando U es grande (necesita una cubeta por valor).
//     RadixSort no, porque solo necesita 256 cubetas en cada pasada.
//
//  Complejidad:
//    Tiempo:   O(d * (n + b))   con b = 256 (base) y d = 4 (bytes)
//              O sea, lineal en n.
//    Espacio:  O(n + b)
// ====================================================================

#ifndef RADIXSORT_H
#define RADIXSORT_H

#include <vector>
#include <cstddef>
#include <cstdint>

namespace radixsort {

    using Llave = unsigned int;

    // Constante: trabajamos en base 256 (un byte por pasada)
    static constexpr int BASE = 256;

    // ----------------------------------------------------------------
    //  pasada_byte()
    //  Hace UN counting sort estable usando el byte numero "indice"
    //  (0 = byte mas bajo, 3 = byte mas alto para uint32).
    //  - entrada : el arreglo actual
    //  - salida  : donde queda el resultado de esta pasada
    //  - indice  : que byte estamos mirando (0..3)
    // ----------------------------------------------------------------
    inline void pasada_byte(const std::vector<Llave>& entrada,
                            std::vector<Llave>& salida,
                            int indice) {

        // Conteo de cuantos numeros tienen cada valor de byte
        std::vector<std::size_t> conteo(BASE, 0);

        // Cuantas veces aparece cada valor de byte (0..255)
        for (std::size_t i = 0; i < entrada.size(); ++i) {
            unsigned int byte = (entrada[i] >> (indice * 8)) & 0xFF;
            conteo[byte]++;
        }

        // Convertimos los conteos en posiciones acumuladas.
        // Asi sabemos donde empieza cada "grupo" en la salida.
        std::size_t acum = 0;
        for (int b = 0; b < BASE; ++b) {
            std::size_t aux = conteo[b];
            conteo[b] = acum;
            acum += aux;
        }

        // Recorremos la entrada en orden y vamos colocando cada
        // numero en su posicion correspondiente. Esto mantiene la
        // estabilidad: si dos numeros tienen el mismo byte, el que
        // venia primero se queda primero.
        for (std::size_t i = 0; i < entrada.size(); ++i) {
            unsigned int byte = (entrada[i] >> (indice * 8)) & 0xFF;
            salida[conteo[byte]] = entrada[i];
            conteo[byte]++;
        }
    }

    // ----------------------------------------------------------------
    //  ordenar()
    //  Hace las 4 pasadas (una por byte) usando un buffer auxiliar.
    //  Al final, copia el resultado de vuelta al arreglo original.
    // ----------------------------------------------------------------
    inline void ordenar(std::vector<Llave>& entrada) {
        if (entrada.empty()) return;

        // Buffer auxiliar del mismo tamaño que la entrada
        std::vector<Llave> buffer(entrada.size());

        // 4 pasadas porque uint32 tiene 4 bytes.
        // Empezamos por el byte menos significativo (indice 0)
        // y terminamos en el mas significativo (indice 3).
        pasada_byte(entrada, buffer, 0);
        pasada_byte(buffer, entrada, 1);
        pasada_byte(entrada, buffer, 2);
        pasada_byte(buffer, entrada, 3);

        // Despues de 4 pasadas el resultado quedo en "entrada"
        // (porque hicimos un numero par de intercambios).
        // Si fuera un numero impar tocaria copiar buffer -> entrada.
    }

    // ----------------------------------------------------------------
    //  ordenar_con_traza()
    //  Version con prints para mostrar como cambia el arreglo
    //  despues de cada pasada de byte.
    // ----------------------------------------------------------------
    template <typename Stream>
    inline void ordenar_con_traza(std::vector<Llave>& entrada, Stream& out) {
        if (entrada.empty()) return;
        std::vector<Llave> buffer(entrada.size());

        for (int byte = 0; byte < 4; ++byte) {
            // alternamos entre entrada y buffer
            if (byte % 2 == 0) {
                pasada_byte(entrada, buffer, byte);
                out << "  tras pasada del byte " << byte << ": ";
                for (Llave v : buffer) out << v << " ";
                out << "\n";
            } else {
                pasada_byte(buffer, entrada, byte);
                out << "  tras pasada del byte " << byte << ": ";
                for (Llave v : entrada) out << v << " ";
                out << "\n";
            }
        }
    }

} // namespace radixsort

#endif
