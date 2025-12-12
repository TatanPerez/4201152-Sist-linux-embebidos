#!/usr/bin/env bash
# Script con comandos útiles para el proyecto Nodo Cisterna ESP32-C6
# Guardar este archivo como: use-me.sh

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# ============================================================================
# COMANDOS RÁPIDOS - NODO DE CISTERNA ESP32-C6
# ============================================================================

echo -e "${BLUE}"
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║    NODO DE SENSOR Y CONTROL DE CISTERNA - COMANDOS ÚTILES     ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo -e "${NC}\n"

# ============================================================================
# 1. COMPILACIÓN
# ============================================================================
compile_project() {
    echo -e "${YELLOW}[Compilando proyecto...]${NC}"
    source ~/esp/esp-idf/export.sh
    idf.py build
}

clean_build() {
    echo -e "${YELLOW}[Limpiando build anterior...]${NC}"
    idf.py fullclean
    echo -e "${GREEN}✓ Build limpio completado${NC}"
}

rebuild_project() {
    clean_build
    compile_project
}

# ============================================================================
# 2. CARGA DE FIRMWARE
# ============================================================================
flash_firmware() {
    local PORT=${1:-"/dev/ttyUSB0"}
    echo -e "${YELLOW}[Cargando firmware en $PORT...]${NC}"
    idf.py -p $PORT flash
}

erase_flash() {
    local PORT=${1:-"/dev/ttyUSB0"}
    echo -e "${YELLOW}[Borrando memoria flash en $PORT...]${NC}"
    idf.py -p $PORT erase-flash
}

# ============================================================================
# 3. MONITOREO
# ============================================================================
monitor_serial() {
    local PORT=${1:-"/dev/ttyUSB0"}
    echo -e "${YELLOW}[Monitoreando puerto $PORT...]${NC}"
    echo -e "${GREEN}Presiona Ctrl+] para salir${NC}"
    idf.py -p $PORT monitor
}

monitor_wifi_only() {
    local PORT=${1:-"/dev/ttyUSB0"}
    echo -e "${YELLOW}[Monitoreando WiFi...]${NC}"
    idf.py -p $PORT monitor | grep -i "wifi"
}

monitor_mqtt_only() {
    local PORT=${1:-"/dev/ttyUSB0"}
    echo -e "${YELLOW}[Monitoreando MQTT...]${NC}"
    idf.py -p $PORT monitor | grep -i "mqtt"
}

monitor_sensors_only() {
    local PORT=${1:-"/dev/ttyUSB0"}
    echo -e "${YELLOW}[Monitoreando sensores...]${NC}"
    idf.py -p $PORT monitor | grep -i "lectura\|nivel\|tds"
}

# ============================================================================
# 4. CONFIGURACIÓN
# ============================================================================
open_menuconfig() {
    echo -e "${YELLOW}[Abriendo menú de configuración...]${NC}"
    source ~/esp/esp-idf/export.sh
    idf.py menuconfig
}

show_config() {
    echo -e "${BLUE}Configuración actual:${NC}"
    grep -E "^CONFIG_" sdkconfig | head -20
}

# ============================================================================
# 5. INFORMACIÓN DEL PROYECTO
# ============================================================================
show_project_structure() {
    echo -e "${BLUE}Estructura del proyecto:${NC}"
    echo ""
    echo "nodo_cisterna/"
    echo "├── main/"
    echo "│   ├── main.c"
    echo "│   └── CMakeLists.txt"
    echo "├── components/"
    echo "│   ├── wifi/          (Conexión WiFi STA)"
    echo "│   ├── mqtt/          (Cliente MQTT)"
    echo "│   ├── sensors/       (Lectura de sensores)"
    echo "│   └── tasks/         (Tareas FreeRTOS)"
    echo "├── CMakeLists.txt"
    echo "├── sdkconfig"
    echo "├── Kconfig.projbuild"
    echo "├── partitions.csv"
    echo "├── README.md"
    echo "├── QUICK_START.md"
    echo "├── CONFIGURACION.md"
    echo "└── ESQUEMA_CONEXIONES.md"
    echo ""
}

list_files() {
    echo -e "${BLUE}Archivos del proyecto:${NC}"
    find . -type f -name "*.c" -o -name "*.h" -o -name "*.txt" -o -name "*.md" | sort
}

# ============================================================================
# 6. MQTT
# ============================================================================
subscribe_mqtt_data() {
    local HOST=${1:-"192.168.1.100"}
    echo -e "${YELLOW}[Suscribiéndose a cistern_sensordata en $HOST...]${NC}"
    mosquitto_sub -h $HOST -t "cistern_sensordata" -v
}

publish_mqtt_on() {
    local HOST=${1:-"192.168.1.100"}
    echo -e "${YELLOW}[Enviando comando ON al topic cistern_control...]${NC}"
    mosquitto_pub -h $HOST -t "cistern_control" -m "ON"
    echo -e "${GREEN}✓ Comando enviado${NC}"
}

publish_mqtt_off() {
    local HOST=${1:-"192.168.1.100"}
    echo -e "${YELLOW}[Enviando comando OFF al topic cistern_control...]${NC}"
    mosquitto_pub -h $HOST -t "cistern_control" -m "OFF"
    echo -e "${GREEN}✓ Comando enviado${NC}"
}

publish_mqtt_auto() {
    local HOST=${1:-"192.168.1.100"}
    echo -e "${YELLOW}[Enviando comando AUTO al topic cistern_control...]${NC}"
    mosquitto_pub -h $HOST -t "cistern_control" -m "AUTO"
    echo -e "${GREEN}✓ Comando enviado${NC}"
}

test_mqtt_connection() {
    local HOST=${1:-"192.168.1.100"}
    echo -e "${YELLOW}[Probando conexión MQTT con $HOST...]${NC}"
    mosquitto_pub -h $HOST -t "test" -m "hello"
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ MQTT accesible${NC}"
    else
        echo -e "${RED}✗ Error conectando a MQTT${NC}"
    fi
}

# ============================================================================
# 7. PUERTOS SERIALES
# ============================================================================
list_serial_ports() {
    echo -e "${BLUE}Puertos seriales disponibles:${NC}"
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        ls -la /dev/tty* | grep -E "USB|ACM"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        ls -la /dev/tty.* | grep -E "USB|usbserial"
    else
        echo "Sistema no soportado para detección automática"
    fi
}

# ============================================================================
# 8. UTILIDADES
# ============================================================================
check_esp_idf() {
    echo -e "${BLUE}Versión ESP-IDF:${NC}"
    idf.py --version
    echo ""
    echo -e "${BLUE}Ruta IDF:${NC}"
    echo $IDF_PATH
}

show_memory_usage() {
    echo -e "${BLUE}Uso de memoria en ESP32-C6:${NC}"
    grep -E "SRAM|IRAM|DRAM" sdkconfig || echo "Información no disponible"
}

show_gpio_usage() {
    echo -e "${BLUE}GPIOs utilizados en este proyecto:${NC}"
    echo "  GPIO-10  → Sensor Ultrasónico TRIG"
    echo "  GPIO-9   → Sensor Ultrasónico ECHO"
    echo "  ADC-0    → Sensor TDS (analógico)"
    echo "  GPIO-8   → Relé Control Bomba"
    echo ""
    echo "Para cambiar, editar main/main.c línea ~230"
}

show_mqtt_topics() {
    echo -e "${BLUE}Topics MQTT configurados:${NC}"
    echo ""
    echo "PUBLICACIÓN (salida de datos):"
    echo "  Topic: cistern_sensordata"
    echo "  QoS: 1"
    echo "  Intervalo: 1 segundo"
    echo "  Formato JSON:"
    echo '    {
      "nivel_agua_cm": 125.50,
      "tds_ppm": 450.2,
      "estado_agua": "MEDIA",
      "estado_bomba": "ON",
      "timestamp": 1234567890
    }'
    echo ""
    echo "SUSCRIPCIÓN (entrada de comandos):"
    echo "  Topic: cistern_control"
    echo "  Comandos: ON, OFF, AUTO"
    echo "  QoS: 1"
}

show_water_classification() {
    echo -e "${BLUE}Clasificación de calidad del agua:${NC}"
    echo "  < 300 ppm   → LIMPIA     (permitir bombeo)"
    echo "  300-600 ppm → MEDIA      (permitir bombeo)"
    echo "  > 600 ppm   → SUCIA      (impedir bombeo)"
}

# ============================================================================
# 9. MENU INTERACTIVO
# ============================================================================
show_menu() {
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${GREEN}MENÚ PRINCIPAL${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
    echo ""
    echo "COMPILACIÓN Y CARGA:"
    echo "  1) Compilar proyecto"
    echo "  2) Compilación limpia"
    echo "  3) Cargar firmware (flash)"
    echo "  4) Borrar memoria flash"
    echo ""
    echo "MONITOREO:"
    echo "  5) Monitor serial completo"
    echo "  6) Monitor solo WiFi"
    echo "  7) Monitor solo MQTT"
    echo "  8) Monitor solo sensores"
    echo ""
    echo "MQTT:"
    echo "  9) Suscribirse a datos"
    echo "  10) Encender bomba (ON)"
    echo "  11) Apagar bomba (OFF)"
    echo "  (Modo automático se implementa en Node-RED mediante la publicación de ON/OFF)"
    echo "  13) Probar conexión MQTT"
    echo ""
    echo "INFORMACIÓN:"
    echo "  14) Estructura del proyecto"
    echo "  15) GPIOs utilizados"
    echo "  16) Topics MQTT"
    echo "  17) Clasificación agua"
    echo "  18) Puertos seriales"
    echo "  19) Versión ESP-IDF"
    echo ""
    echo "  20) Salir"
    echo ""
}

interactive_menu() {
    while true; do
        show_menu
        read -p "Selecciona opción [1-20]: " choice
        
        case $choice in
            1) compile_project ;;
            2) clean_build ;;
            3) flash_firmware ;;
            4) erase_flash ;;
            5) monitor_serial ;;
            6) monitor_wifi_only ;;
            7) monitor_mqtt_only ;;
            8) monitor_sensors_only ;;
            9) subscribe_mqtt_data ;;
            10) publish_mqtt_on ;;
            11) publish_mqtt_off ;;
            # 12 (Modo automático) ya no es soportado por firmware; impleméntalo en Node-RED
            13) test_mqtt_connection ;;
            14) show_project_structure ;;
            15) show_gpio_usage ;;
            16) show_mqtt_topics ;;
            17) show_water_classification ;;
            18) list_serial_ports ;;
            19) check_esp_idf ;;
            20) echo -e "${GREEN}¡Hasta luego!${NC}"; exit 0 ;;
            *) echo -e "${RED}Opción inválida${NC}" ;;
        esac
        
        echo ""
        read -p "Presiona Enter para continuar..."
        clear
    done
}

# ============================================================================
# EJECUCIÓN
# ============================================================================

if [ "$1" == "" ]; then
    # Sin argumentos: mostrar menú interactivo
    interactive_menu
else
    # Con argumentos: ejecutar comando específico
    case "$1" in
        "build") compile_project ;;
        "clean") clean_build ;;
        "rebuild") rebuild_project ;;
        "flash") flash_firmware "$2" ;;
        "erase") erase_flash "$2" ;;
        "monitor") monitor_serial "$2" ;;
        "mqtt-sub") subscribe_mqtt_data "$2" ;;
        "mqtt-on") publish_mqtt_on "$2" ;;
        "mqtt-off") publish_mqtt_off "$2" ;;
        # mqtt-auto removed; use Node-RED automation publishing ON/OFF
        "mqtt-test") test_mqtt_connection "$2" ;;
        "status") check_esp_idf ;;
        "structure") show_project_structure ;;
        "gpio") show_gpio_usage ;;
        "mqtt") show_mqtt_topics ;;
        "water") show_water_classification ;;
        "ports") list_serial_ports ;;
        "config") open_menuconfig ;;
        "menu") interactive_menu ;;
        *)
            echo -e "${RED}Comando no reconocido: $1${NC}"
            echo ""
            echo "Uso: $0 [comando] [args]"
            echo ""
            echo "Comandos disponibles:"
            echo "  build              - Compilar proyecto"
            echo "  clean              - Limpiar build"
            echo "  rebuild            - Limpiar y compilar"
            echo "  flash [puerto]     - Cargar firmware (default: /dev/ttyUSB0)"
            echo "  erase [puerto]     - Borrar memoria flash"
            echo "  monitor [puerto]   - Monitor serial"
            echo "  mqtt-sub [host]    - Suscribirse a datos MQTT"
            echo "  mqtt-on [host]     - Encender bomba"
            echo "  mqtt-off [host]    - Apagar bomba"
            echo "  mqtt-auto [host]   - Modo automático"
            echo "  mqtt-test [host]   - Probar MQTT"
            echo "  status             - Ver información ESP-IDF"
            echo "  structure          - Ver estructura proyecto"
            echo "  gpio               - Ver GPIOs utilizados"
            echo "  mqtt               - Ver topics MQTT"
            echo "  water              - Ver clasificación agua"
            echo "  ports              - Ver puertos seriales"
            echo "  config             - Abrir menuconfig"
            echo "  menu               - Mostrar menú interactivo"
            exit 1
            ;;
    esac
fi
