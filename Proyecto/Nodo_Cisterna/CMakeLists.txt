# CMakeLists.txt para proyecto ESP-IDF de Nodo de Cisterna
# Proyecto: Sistema de monitoreo y control de cisterna con ESP32-C6

cmake_minimum_required(VERSION 3.16)

# Incluir el toolchain de ESP-IDF y definir el proyecto
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Nodo_Cisterna)

# Opciones de compilación adicionales
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
