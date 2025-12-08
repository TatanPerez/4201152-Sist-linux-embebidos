# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/tatan/esp/esp-idf/components/bootloader/subproject"
  "/mnt/c/Users/tatan/Documentos/UNAL/8_License_Plate/Linux/4201152-Sist-linux-embebidos/Proyecto/Node_Tank/build/bootloader"
  "/mnt/c/Users/tatan/Documentos/UNAL/8_License_Plate/Linux/4201152-Sist-linux-embebidos/Proyecto/Node_Tank/build/bootloader-prefix"
  "/mnt/c/Users/tatan/Documentos/UNAL/8_License_Plate/Linux/4201152-Sist-linux-embebidos/Proyecto/Node_Tank/build/bootloader-prefix/tmp"
  "/mnt/c/Users/tatan/Documentos/UNAL/8_License_Plate/Linux/4201152-Sist-linux-embebidos/Proyecto/Node_Tank/build/bootloader-prefix/src/bootloader-stamp"
  "/mnt/c/Users/tatan/Documentos/UNAL/8_License_Plate/Linux/4201152-Sist-linux-embebidos/Proyecto/Node_Tank/build/bootloader-prefix/src"
  "/mnt/c/Users/tatan/Documentos/UNAL/8_License_Plate/Linux/4201152-Sist-linux-embebidos/Proyecto/Node_Tank/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/mnt/c/Users/tatan/Documentos/UNAL/8_License_Plate/Linux/4201152-Sist-linux-embebidos/Proyecto/Node_Tank/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/mnt/c/Users/tatan/Documentos/UNAL/8_License_Plate/Linux/4201152-Sist-linux-embebidos/Proyecto/Node_Tank/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
