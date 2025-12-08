#!/bin/bash
# Script de instalación y compilación para WSL Ubuntu
# Nodo de Sensor y Control de Cisterna

set -e  # Salir si hay error

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}================================${NC}"
echo -e "${GREEN}Instalación ESP-IDF y Proyecto${NC}"
echo -e "${GREEN}Nodo de Cisterna${NC}"
echo -e "${GREEN}================================${NC}\n"

# 1. Verificar WSL Ubuntu
echo -e "${YELLOW}[1/6] Verificando WSL Ubuntu...${NC}"
if [[ -f /etc/os-release ]]; then
    source /etc/os-release
    if [[ "$ID" == "ubuntu" ]]; then
        echo -e "${GREEN}✓ WSL Ubuntu detectado: $VERSION_ID${NC}\n"
    else
        echo -e "${RED}✗ No es Ubuntu. Se require Ubuntu en WSL${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}⚠ No se puede verificar sistema${NC}\n"
fi

# 2. Actualizar sistema e instalar dependencias
echo -e "${YELLOW}[2/6] Actualizando sistema e instalando dependencias...${NC}"
sudo apt update
sudo apt install -y \
    git wget flex bison gperf python3 python3-venv cmake ninja-build ccache \
    libffi-dev libssl-dev build-essential libtool pkg-config autoconf automake

echo -e "${GREEN}✓ Dependencias instaladas${NC}\n"

# 3. Crear directorio de trabajo
echo -e "${YELLOW}[3/6] Creando directorio de trabajo...${NC}"
mkdir -p ~/esp
cd ~/esp
echo -e "${GREEN}✓ Directorio creado en ~/esp${NC}\n"

# 4. Instalar ESP-IDF
echo -e "${YELLOW}[4/6] Instalando ESP-IDF...${NC}"
if [ ! -d "esp-idf" ]; then
    echo "Clonando ESP-IDF..."
    git clone --recursive --depth=1 https://github.com/espressif/esp-idf.git
    cd esp-idf
    echo "Ejecutando instalador..."
    ./install.sh esp32c6
    cd ..
    echo -e "${GREEN}✓ ESP-IDF instalado${NC}\n"
else
    echo -e "${YELLOW}⚠ ESP-IDF ya existe en ~/esp/esp-idf${NC}\n"
fi

# 5. Configurar variables de entorno
echo -e "${YELLOW}[5/6] Configurando variables de entorno...${NC}"
if ! grep -q "IDF_PATH" ~/.bashrc; then
    cat >> ~/.bashrc << 'EOF'

# ESP-IDF Configuration
export IDF_PATH=~/esp/esp-idf
alias get_idf='. $IDF_PATH/export.sh'
EOF
    echo -e "${GREEN}✓ Variables agregadas a ~/.bashrc${NC}"
    source ~/.bashrc
else
    echo -e "${YELLOW}⚠ Variables de entorno ya configuradas${NC}\n"
fi

# 6. Compilar proyecto
echo -e "${YELLOW}[6/6] Configurando proyecto Nodo Cisterna...${NC}"
if [ -d "nodo_cisterna" ]; then
    cd nodo_cisterna
    source ~/esp/esp-idf/export.sh
    idf.py set-target esp32c6
    echo -e "${GREEN}✓ Proyecto configurado para ESP32-C6${NC}\n"
    
    echo -e "${GREEN}================================${NC}"
    echo -e "${GREEN}✓ INSTALACIÓN COMPLETADA${NC}"
    echo -e "${GREEN}================================${NC}\n"
    
    echo -e "${YELLOW}Próximos pasos:${NC}"
    echo "1. Configurar credenciales Wi-Fi y MQTT en main/main.c"
    echo "2. Compilar: idf.py build"
    echo "3. Cargar: idf.py -p /dev/ttyUSB0 flash"
    echo "4. Monitor: idf.py -p /dev/ttyUSB0 monitor"
    echo ""
    echo -e "${YELLOW}Para activar variables de entorno en nueva terminal:${NC}"
    echo "source ~/esp/esp-idf/export.sh"
    echo ""
else
    echo -e "${RED}✗ Directorio nodo_cisterna no encontrado${NC}"
    exit 1
fi
