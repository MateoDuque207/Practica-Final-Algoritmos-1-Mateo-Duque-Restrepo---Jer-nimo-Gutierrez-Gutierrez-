// ====================================================================
//  gui.cpp
// --------------------------------------------------------------------
//  Interfaz grafica de la Practica II usando SFML.
//
//  La idea es bien sencilla:
//   - Una ventana con unos botones arriba.
//   - El area grande del medio dibuja el arreglo como barras.
//   - Abajo hay una linea de info con el algoritmo y el tiempo.
//
//  No es nada del otro mundo, lo importante es poder mostrar al profe
//  el comportamiento de los algoritmos sin tener que mirar consola.
//
//  Como funciona la animacion:
//   1. Al darle "Ordenar", corremos la version "_pasos" del algoritmo,
//      que devuelve un vector de snapshots (uno por cada operacion).
//   2. Despues, en el loop principal, vamos avanzando un snapshot
//      cada N milisegundos y redibujamos.
//   3. Aparte, medimos el tiempo del algoritmo "real" (no el de
//      pasos) sobre el mismo arreglo para mostrar el tiempo verdadero.
// ====================================================================

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <random>
#include <algorithm>

#include "dialsort.h"
#include "radixsort.h"
#include "stlsort.h"
#include "generador.h"
#include "pasos.h"

using Llave = unsigned int;

// --------------- constantes de la ventana -------------------------
static const int VENTANA_W   = 1000;
static const int VENTANA_H   = 650;
static const int AREA_BARRAS_Y      = 130;       // donde empiezan las barras
static const int AREA_BARRAS_ALTURA = 420;       // altura del area de barras
static const int N_VISUAL    = 50;               // numeros para visualizar
static const int U_VISUAL    = 50;               // universo para visualizar

// --------------- helper: cargar fuente ----------------------------
//  Probamos varias rutas comunes. Si ninguna funciona, devolvemos
//  false y la GUI sigue funcionando solo con formas (sin texto).
static bool cargar_fuente(sf::Font& f) {
    const char* rutas[] = {
        "arial.ttf",                                                // misma carpeta
        "DejaVuSans.ttf",                                           // misma carpeta
        "C:/Windows/Fonts/arial.ttf",                               // Windows
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",          // Ubuntu/Debian
        "/usr/share/fonts/TTF/DejaVuSans.ttf",                      // Arch
        "/System/Library/Fonts/Supplemental/Arial.ttf",             // macOS
        "/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf"
    };
    for (const char* r : rutas) {
        if (f.loadFromFile(r)) return true;
    }
    return false;
}

// --------------- estructura simple de boton -----------------------
struct Boton {
    sf::RectangleShape rect;
    sf::Text texto;
    bool activo = false;   // si esta resaltado (algoritmo seleccionado, p.ej.)

    bool contiene(sf::Vector2f p) const {
        return rect.getGlobalBounds().contains(p);
    }
};

// helper para hacer un boton de manera rapida
static Boton hacer_boton(float x, float y, float w, float h,
                         const std::string& etiqueta, sf::Font& f) {
    Boton b;
    b.rect.setPosition(x, y);
    b.rect.setSize({w, h});
    b.rect.setFillColor(sf::Color(60, 60, 80));
    b.rect.setOutlineThickness(2);
    b.rect.setOutlineColor(sf::Color(120, 120, 140));

    b.texto.setFont(f);
    b.texto.setString(etiqueta);
    b.texto.setCharacterSize(14);
    b.texto.setFillColor(sf::Color::White);
    // centrar mas o menos
    sf::FloatRect r = b.texto.getLocalBounds();
    b.texto.setPosition(x + (w - r.width) / 2 - r.left,
                        y + (h - r.height) / 2 - r.top - 2);
    return b;
}

// ----------------------------------------------------------------
//  Estado global del visualizador
// ----------------------------------------------------------------
enum class Algoritmo { DIALSORT, RADIXSORT, STDSORT };

struct Estado {
    std::vector<Llave> datos;
    std::vector<pasos::Snapshot> snapshots;
    int paso_actual = 0;
    bool reproduciendo = false;
    Algoritmo algoritmo = Algoritmo::DIALSORT;
    double tiempo_real_seg = 0.0;       // tiempo del algoritmo de verdad
    int velocidad_ms = 50;              // ms entre snapshots
    sf::Clock reloj_animacion;
    std::string mensaje_actual;
};

// ----------------------------------------------------------------
//  Genera un dataset chiquito para visualizar
// ----------------------------------------------------------------
static void regenerar_datos(Estado& est, generador::Distribucion d) {
    static std::uint64_t semilla = 1;
    semilla++;
    est.datos = generador::generar(N_VISUAL, U_VISUAL, d, semilla);
    est.snapshots.clear();
    est.snapshots.push_back({est.datos, -1, "Listo para ordenar"});
    est.paso_actual = 0;
    est.reproduciendo = false;
    est.tiempo_real_seg = 0.0;
    est.mensaje_actual = "Listo para ordenar";
}

// ----------------------------------------------------------------
//  Corre el algoritmo elegido y guarda los snapshots, y aparte
//  mide el tiempo real con la version "rapida" (sin snapshots).
// ----------------------------------------------------------------
static void empezar_ordenamiento(Estado& est) {
    auto copia_para_medir = est.datos;

    // Medimos el tiempo real
    auto t0 = std::chrono::steady_clock::now();
    if (est.algoritmo == Algoritmo::DIALSORT) {
        dialsort::ordenar(copia_para_medir, U_VISUAL);
    } else if (est.algoritmo == Algoritmo::RADIXSORT) {
        radixsort::ordenar(copia_para_medir);
    } else {
        stlsort::ordenar(copia_para_medir);
    }
    auto t1 = std::chrono::steady_clock::now();
    std::chrono::duration<double> dt = t1 - t0;
    est.tiempo_real_seg = dt.count();

    // Generamos los snapshots para la animacion
    if (est.algoritmo == Algoritmo::DIALSORT) {
        est.snapshots = pasos::dialsort_pasos(est.datos, U_VISUAL);
    } else if (est.algoritmo == Algoritmo::RADIXSORT) {
        est.snapshots = pasos::radixsort_pasos(est.datos);
    } else {
        // std::sort es black box, solo mostramos antes y despues
        pasos::Snapshot inicial;
        inicial.arr = est.datos;
        inicial.resaltado = -1;
        inicial.mensaje = "std::sort: black box, solo mostramos resultado";

        auto copia = est.datos;
        std::sort(copia.begin(), copia.end());

        pasos::Snapshot final;
        final.arr = copia;
        final.resaltado = -1;
        final.mensaje = "std::sort termino";

        est.snapshots = {inicial, final};
    }

    est.paso_actual = 0;
    est.reproduciendo = true;
    est.reloj_animacion.restart();
}

// ----------------------------------------------------------------
//  Dibuja las barras del arreglo en el area central
// ----------------------------------------------------------------
static void dibujar_barras(sf::RenderWindow& w, const pasos::Snapshot& snap) {
    if (snap.arr.empty()) return;
    int n = static_cast<int>(snap.arr.size());

    // ancho disponible y de cada barra
    float ancho_total = static_cast<float>(VENTANA_W - 40);
    float ancho_barra = ancho_total / static_cast<float>(n);
    float gap = 1.0f;

    // valor maximo para escalar la altura
    Llave max_v = 1;
    for (Llave v : snap.arr) if (v > max_v) max_v = v;

    for (int i = 0; i < n; ++i) {
        float h = (static_cast<float>(snap.arr[i]) / static_cast<float>(max_v))
                  * AREA_BARRAS_ALTURA;
        float x = 20 + i * ancho_barra;
        float y = AREA_BARRAS_Y + AREA_BARRAS_ALTURA - h;

        sf::RectangleShape barra;
        barra.setPosition(x, y);
        barra.setSize({ancho_barra - gap, h});

        if (i == snap.resaltado) {
            barra.setFillColor(sf::Color(230, 100, 100));   // rojo si resaltada
        } else {
            // un degradado segun el valor (azul -> verde)
            int v = static_cast<int>(snap.arr[i]);
            int azul = 180 - (v * 130 / static_cast<int>(max_v));
            int verde = 100 + (v * 130 / static_cast<int>(max_v));
            barra.setFillColor(sf::Color(80, verde, azul));
        }
        w.draw(barra);
    }
}

// ----------------------------------------------------------------
//  main de la GUI
// ----------------------------------------------------------------
int main() {
    sf::RenderWindow ventana(sf::VideoMode(VENTANA_W, VENTANA_H),
                             "Practica II - Visualizador de Ordenamientos");
    ventana.setFramerateLimit(60);

    sf::Font fuente;
    bool hay_fuente = cargar_fuente(fuente);
    if (!hay_fuente) {
        std::cerr << "No se encontro ninguna fuente, el texto no se vera.\n";
        // creamos una "fuente vacia" para que no falle el constructor de Text
    }

    // ---------- crear los botones ---------------------------------
    Boton btn_dialsort  = hacer_boton( 20, 20, 110, 35, "DialSort",  fuente);
    Boton btn_radix     = hacer_boton(140, 20, 110, 35, "RadixSort", fuente);
    Boton btn_stdsort   = hacer_boton(260, 20, 110, 35, "std::sort", fuente);
    btn_dialsort.activo = true;     // por defecto

    Boton btn_generar   = hacer_boton(420, 20, 130, 35, "Generar nuevo",  fuente);
    Boton btn_ordenar   = hacer_boton(560, 20, 110, 35, "Ordenar",        fuente);

    Boton btn_uniforme  = hacer_boton( 20, 75,  90, 30, "Uniforme",     fuente);
    Boton btn_casiord   = hacer_boton(120, 75, 110, 30, "Casi-ordenado", fuente);
    Boton btn_duplicad  = hacer_boton(240, 75, 100, 30, "Duplicados",   fuente);
    Boton btn_ascend    = hacer_boton(350, 75,  90, 30, "Ascendente",   fuente);
    Boton btn_descend   = hacer_boton(450, 75,  90, 30, "Descendente",  fuente);
    btn_uniforme.activo = true;

    Boton btn_lento     = hacer_boton(680, 75,  60, 30, "Lento",  fuente);
    Boton btn_normal    = hacer_boton(745, 75,  60, 30, "Normal", fuente);
    Boton btn_rapido    = hacer_boton(810, 75,  60, 30, "Rapido", fuente);
    btn_normal.activo = true;

    // ---------- estado inicial ------------------------------------
    Estado est;
    generador::Distribucion dist_actual = generador::Distribucion::UNIFORME;
    regenerar_datos(est, dist_actual);

    // helpers para activar/desactivar botones de un grupo
    auto activar_solo = [](std::vector<Boton*> grupo, Boton* el) {
        for (auto b : grupo) b->activo = false;
        el->activo = true;
    };

    // ---------- loop principal ------------------------------------
    while (ventana.isOpen()) {
        sf::Event ev;
        while (ventana.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) ventana.close();

            if (ev.type == sf::Event::MouseButtonPressed &&
                ev.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f p(static_cast<float>(ev.mouseButton.x),
                               static_cast<float>(ev.mouseButton.y));

                // botones de algoritmo
                if (btn_dialsort.contiene(p)) {
                    activar_solo({&btn_dialsort, &btn_radix, &btn_stdsort},
                                 &btn_dialsort);
                    est.algoritmo = Algoritmo::DIALSORT;
                } else if (btn_radix.contiene(p)) {
                    activar_solo({&btn_dialsort, &btn_radix, &btn_stdsort},
                                 &btn_radix);
                    est.algoritmo = Algoritmo::RADIXSORT;
                } else if (btn_stdsort.contiene(p)) {
                    activar_solo({&btn_dialsort, &btn_radix, &btn_stdsort},
                                 &btn_stdsort);
                    est.algoritmo = Algoritmo::STDSORT;
                }

                // botones de distribucion
                else if (btn_uniforme.contiene(p)) {
                    activar_solo({&btn_uniforme, &btn_casiord, &btn_duplicad,
                                  &btn_ascend, &btn_descend}, &btn_uniforme);
                    dist_actual = generador::Distribucion::UNIFORME;
                    regenerar_datos(est, dist_actual);
                } else if (btn_casiord.contiene(p)) {
                    activar_solo({&btn_uniforme, &btn_casiord, &btn_duplicad,
                                  &btn_ascend, &btn_descend}, &btn_casiord);
                    dist_actual = generador::Distribucion::CASI_ORDENADO;
                    regenerar_datos(est, dist_actual);
                } else if (btn_duplicad.contiene(p)) {
                    activar_solo({&btn_uniforme, &btn_casiord, &btn_duplicad,
                                  &btn_ascend, &btn_descend}, &btn_duplicad);
                    dist_actual = generador::Distribucion::DUPLICADOS;
                    regenerar_datos(est, dist_actual);
                } else if (btn_ascend.contiene(p)) {
                    activar_solo({&btn_uniforme, &btn_casiord, &btn_duplicad,
                                  &btn_ascend, &btn_descend}, &btn_ascend);
                    dist_actual = generador::Distribucion::ASCENDENTE;
                    regenerar_datos(est, dist_actual);
                } else if (btn_descend.contiene(p)) {
                    activar_solo({&btn_uniforme, &btn_casiord, &btn_duplicad,
                                  &btn_ascend, &btn_descend}, &btn_descend);
                    dist_actual = generador::Distribucion::DESCENDENTE;
                    regenerar_datos(est, dist_actual);
                }

                // botones de velocidad
                else if (btn_lento.contiene(p)) {
                    activar_solo({&btn_lento, &btn_normal, &btn_rapido},
                                 &btn_lento);
                    est.velocidad_ms = 150;
                } else if (btn_normal.contiene(p)) {
                    activar_solo({&btn_lento, &btn_normal, &btn_rapido},
                                 &btn_normal);
                    est.velocidad_ms = 50;
                } else if (btn_rapido.contiene(p)) {
                    activar_solo({&btn_lento, &btn_normal, &btn_rapido},
                                 &btn_rapido);
                    est.velocidad_ms = 10;
                }

                // botones de accion
                else if (btn_generar.contiene(p)) {
                    regenerar_datos(est, dist_actual);
                } else if (btn_ordenar.contiene(p)) {
                    empezar_ordenamiento(est);
                }
            }
        }

        // ---------- avanzar la animacion ---------------------------
        if (est.reproduciendo &&
            est.paso_actual < static_cast<int>(est.snapshots.size()) - 1) {
            if (est.reloj_animacion.getElapsedTime().asMilliseconds() >=
                est.velocidad_ms) {
                est.paso_actual++;
                est.mensaje_actual = est.snapshots[est.paso_actual].mensaje;
                est.reloj_animacion.restart();
            }
        } else {
            est.reproduciendo = false;
        }

        // ---------- dibujar ---------------------------------------
        ventana.clear(sf::Color(30, 30, 40));

        // pintar los botones
        std::vector<Boton*> todos = {
            &btn_dialsort, &btn_radix, &btn_stdsort,
            &btn_generar, &btn_ordenar,
            &btn_uniforme, &btn_casiord, &btn_duplicad, &btn_ascend, &btn_descend,
            &btn_lento, &btn_normal, &btn_rapido
        };
        for (Boton* b : todos) {
            if (b->activo) {
                b->rect.setFillColor(sf::Color(80, 130, 180));
            } else {
                b->rect.setFillColor(sf::Color(60, 60, 80));
            }
            ventana.draw(b->rect);
            if (hay_fuente) ventana.draw(b->texto);
        }

        // separador visual
        sf::RectangleShape linea;
        linea.setPosition(0, 115);
        linea.setSize({static_cast<float>(VENTANA_W), 2.f});
        linea.setFillColor(sf::Color(80, 80, 100));
        ventana.draw(linea);

        // dibujar las barras
        if (!est.snapshots.empty() &&
            est.paso_actual < static_cast<int>(est.snapshots.size())) {
            dibujar_barras(ventana, est.snapshots[est.paso_actual]);
        }

        // panel de info abajo
        sf::RectangleShape panel;
        panel.setPosition(0, 560);
        panel.setSize({static_cast<float>(VENTANA_W), 90.f});
        panel.setFillColor(sf::Color(40, 40, 55));
        ventana.draw(panel);

        if (hay_fuente) {
            sf::Text info;
            info.setFont(fuente);
            info.setCharacterSize(15);
            info.setFillColor(sf::Color::White);

            std::string nombre_alg;
            switch (est.algoritmo) {
                case Algoritmo::DIALSORT:  nombre_alg = "DialSort";  break;
                case Algoritmo::RADIXSORT: nombre_alg = "RadixSort"; break;
                case Algoritmo::STDSORT:   nombre_alg = "std::sort"; break;
            }

            char buf[256];
            std::snprintf(buf, sizeof(buf),
                          "Algoritmo: %s    n: %d    Tiempo real: %.6f s",
                          nombre_alg.c_str(), N_VISUAL, est.tiempo_real_seg);
            info.setString(buf);
            info.setPosition(20, 575);
            ventana.draw(info);

            sf::Text msg;
            msg.setFont(fuente);
            msg.setCharacterSize(13);
            msg.setFillColor(sf::Color(180, 220, 255));
            msg.setString(est.mensaje_actual);
            msg.setPosition(20, 605);
            ventana.draw(msg);

            sf::Text contador;
            contador.setFont(fuente);
            contador.setCharacterSize(13);
            contador.setFillColor(sf::Color(150, 150, 170));
            std::snprintf(buf, sizeof(buf), "Paso %d / %d",
                          est.paso_actual + 1,
                          static_cast<int>(est.snapshots.size()));
            contador.setString(buf);
            contador.setPosition(VENTANA_W - 150, 605);
            ventana.draw(contador);
        }

        ventana.display();
    }

    return 0;
}
