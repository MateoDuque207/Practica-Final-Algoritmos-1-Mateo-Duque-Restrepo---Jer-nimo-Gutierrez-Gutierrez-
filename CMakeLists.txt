cmake_minimum_required(VERSION 3.10)
project(dialsort_practica2 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compilamos en Release por defecto para que las mediciones sean justas
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
    endif()
endif()

# ---- ejecutable de consola (benchmark, simulacion, corrida personalizada)
add_executable(practica2
    main.cpp
    dialsort.h
    radixsort.h
    stlsort.h
    generador.h
    medidor.h
    visual.h
)

# ---- ejecutable con interfaz grafica (SFML)
# Si SFML no esta instalado, este target simplemente no se construye y
# practica2 (consola) sigue funcionando normalmente.
find_package(SFML 2.5 COMPONENTS graphics window system QUIET)

if(SFML_FOUND)
    add_executable(practica2_gui
        gui.cpp
        dialsort.h
        radixsort.h
        stlsort.h
        generador.h
        medidor.h
        pasos.h
    )
    target_link_libraries(practica2_gui PRIVATE
        sfml-graphics sfml-window sfml-system)
    message(STATUS "SFML encontrado, se construira practica2_gui")
else()
    message(STATUS "SFML no encontrado, solo se construira practica2 (consola)")
endif()
