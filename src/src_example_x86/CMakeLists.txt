# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(HelloWorldX86 CXX)

# Añade el ejecutable, indicando que las fuentes están en la carpeta src
add_executable(hello_world_x86 main_example_x86.cpp)

# 1. Indicamos al enlazador que use versiones estáticas de las librerías estándar
target_link_options(hello_world_x86 PRIVATE
    -static
)

