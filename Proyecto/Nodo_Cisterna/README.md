# NODO DE SENSOR Y CONTROL DE CISTERNA CON ESP32-C6

## Descripción General

Sistema IoT completo para monitoreo y control automático de una cisterna de agua. Utiliza sensores ultrasónicos y TDS para determinar nivel y calidad del agua, controla una bomba sumergible mediante un relé HW-307, y se comunica con un broker MQTT para transmitir datos y recibir comandos.

### Características
- **Monitoreo en tiempo real** de nivel y calidad del agua
- **Control automático** de bomba sumergible basado en umbrales
- **Control manual** de bomba mediante comandos MQTT
- **Comunicación inalámbrica** vía Wi-Fi y MQTT
- **Procesamiento concurrente** con tareas FreeRTOS
- **Sincronización segura** con semáforos

---

## Requisitos de Hardware

### Componentes Principales
1. **ESP32-C6** - Microcontrolador principal
2. **Sensor Ultrasónico HC-SR04** - Medición de nivel de agua
3. **Sensor TDS** - Medición de calidad del agua (conductividad)
4. **Relé HW-307** - Control de bomba sumergible
5. **Bomba Sumergible** - Control de flujo de agua

### Pines GPIO Utilizados
```
Pin 10 → Sensor Ultrasónico TRIG
Pin 9  → Sensor Ultrasónico ECHO
ADC0   → Sensor TDS (análogo)
Pin 8  → Relé HW-307 (control de bomba)
```

### Conectividad
- **Wi-Fi**: Integrada en ESP32-C6
- **MQTT**: Broker local (ej: Mosquitto en Raspberry Pi)

---

## Estructura del Proyecto

```
nodo_cisterna/
├── main/
│   ├── main.c                 # Entrada de la aplicación, creación de tareas y configuración de periféricos
│   ├── port_compat.c          # Adaptaciones de plataforma/compatibilidad (si aplica)
│   └── CMakeLists.txt         # Objetivo del componente `app`
├── components/
│   ├── wifi/
│   │   ├── wifi.h             # Interfaz y API pública para inicializar/gestionar Wi‑Fi
│   │   ├── wifi.c             # Implementación: conexión, eventos y diagnósticos
│   │   └── CMakeLists.txt
│   ├── mqtt/                  # Wrapper local para publicar/suscribirse (renombrado a evitar colisión con IDF)
│   │   ├── mqtt.h             # API para conectar/publicar/suscribirse
│   │   ├── mqtt.c             # Implementación cliente MQTT (usa IDF MQTT internamente)
│   │   └── CMakeLists.txt
│   ├── sensors/
│   │   ├── sensor.h           # API de sensores (ultrasonido, TDS) y funciones de calibración
│   │   ├── sensor.c           # Implementaciones de lectura, formatos JSON y comandos UART
│   │   └── CMakeLists.txt
│   ├── tds/
│   │   ├── tds.h              # Lógica de conversión raw→ppm y manejo de calibración
│   │   ├── tds.c              # Cálculo de offset/gain, save/load en NVS
│   │   └── CMakeLists.txt
│   ├── adc_driver/
│   │   ├── adc_driver.h       # Wrappers de ADC (lecturas, muestras, voltaje)
│   │   ├── adc_driver.c       # Implementación específica del ADC usado (oneshot / muestras)
│   │   └── CMakeLists.txt
│   ├── storage/
│   │   ├── storage.h          # API simple para persistencia en NVS
│   │   ├── storage.c          # Implementación de lectura/escritura de claves (tds_offset, tds_gain)
│   │   └── CMakeLists.txt
│   ├── tasks/
│   │   ├── tasks.h            # Definición de configuraciones y prototipos de tareas
│   │   ├── tasks.c            # Tasks auxiliares si las hubiera
│   │   └── CMakeLists.txt
│   └── misc/                  # Otros componentes auxiliares (p. ej. drivers específicos)
│       └── CMakeLists.txt
├── build/                     # Directorio de salida de CMake/IDF (no commitear)
├── CMakeLists.txt             # Configuración principal del proyecto (incluye componentes)
├── sdkconfig                  # Archivo de configuración generado por `idf.py menuconfig`
├── Kconfig.projbuild          # Opciones de configuración del proyecto
├── partitions.csv             # Tabla de particiones para el flash
├── scripts/                   # (opcional) scripts útiles (flash, formateo, snapshot)
├── comandos-utiles.sh         # Script con comandos útiles del desarrollador
├── setup_wsl.sh               # Scripts de preparación para WSL (si aplica)
└── README.md                  # Este archivo
```

Descripción breve de carpetas y archivos clave:

- `main/main.c`: configura periféricos (UART, ADC, GPIO), inicializa `nvs_flash`, Wi‑Fi, MQTT, y crea las tareas FreeRTOS principales: la tarea de lectura/publicación de sensores y el lector UART para comandos (`uart_command_task`).
- `components/wifi/`: encapsula la lógica de conexión Wi‑Fi, eventos y diagnósticos (se agregaron logs de razón de desconexión para depuración).
- `components/mqtt/`: wrapper local que evita colisiones con el componente `mqtt` del ESP-IDF — expone funciones sencillas para publicar JSON y gestionar la conexión.
- `components/sensors/`: incluye lecturas de ultrasonido y TDS; aquí también se exponen `sensor_do_calA/B/save/show` que son llamadas por el lector UART para calibración.
- `components/tds/`: contiene la lógica de conversión raw→ppm y las funciones para establecer/calcular `offset` y `gain`, además de persistirlos en `storage`.
- `components/adc_driver/`: centraliza la lectura ADC (muestras, promediado, conversión a voltaje) para facilitar cambios de hardware.
- `components/storage/`: capa pequeña sobre NVS para guardar claves como `tds_offset` y `tds_gain`.

Archivos y utilidades fuera del árbol de código fuente:

- `comandos-utiles.sh`: colección de comandos útiles para build/flash/monitorización.
- `setup_wsl.sh`: pasos automatizados para configurar ESP-IDF en WSL (útil para desarrolladores en Windows).

Si necesitas, puedo añadir aquí un diagrama simple de dependencias entre componentes (por ejemplo: `main -> sensors -> tds -> storage`, `main -> wifi -> mqtt`).

---

## Flujo de Datos y Control

### 1. Inicialización
```
1. NVS Flash (almacenamiento no volátil)
2. Wi-Fi (conexión a red)
3. MQTT (conexión a broker)
4. Sensores (inicialización de GPIO y ADC)
5. Tareas FreeRTOS (lectura periódica)
```

### 2. Ciclo de Lectura y Control (cada 1 segundo)
```
1. Leer sensor ultrasónico → nivel de agua
2. Leer sensor TDS → calidad del agua
3. Clasificar calidad (limpia/media/sucia)
4. Aplicar lógica automática de bomba (si no está en override manual)
5. Publicar datos en topic "cistern_sensordata" (JSON)
6. Esperar siguiente ciclo
```

### 3. Lógica de Control de Bomba
```
AUTOMÁTICO:
  Si (nivel_bajo < 20cm) AND (agua ≤ 600 ppm)
    → Encender bomba
  Si (nivel_alto > 180cm) OR (agua > 600 ppm)
    → Apagar bomba

MANUAL (vía MQTT "cistern_control"):
  "ON"   → Encender bomba
  "OFF"  → Apagar bomba
  "AUTO" → Volver a automático
```

---

## Temas MQTT

### Publicación (datos del sensor)
**Topic:** `cistern_sensordata`

**Formato JSON:**
```json
{
  "nivel_agua_cm": 125.5,
  "tds_ppm": 450.2,
  "estado_agua": "MEDIA",
  "estado_bomba": "ON",
  "timestamp": 1234567890
}
```

**Frecuencia:** Cada 1 segundo

### Suscripción (comandos de control)
**Topic:** `cistern_control`

**Valores aceptados:**
- `ON` - Encender bomba manualmente
- `OFF` - Apagar bomba manualmente
- `AUTO` - Activar control automático

---

## Clasificación de Calidad del Agua

| TDS (ppm) | Estado | Descripción |
|-----------|--------|-------------|
| < 300    | LIMPIA | Agua aceptable para bombeo |
| 300-600  | MEDIA  | Agua con conductividad media |
| > 600    | SUCIA  | Agua contaminada, bomba se desactiva |

---

## Instalación y Compilación

### Requisitos Previos
- **ESP-IDF** instalado en WSL Ubuntu
- **Python** 3.7 o superior
- **git** para control de versiones

### Pasos de Instalación

#### 1. Preparar entorno WSL Ubuntu
```bash
# Actualizar sistema
sudo apt update && sudo apt upgrade -y

# Instalar dependencias
sudo apt install -y git wget flex bison gperf python3 python3-venv cmake ninja-build ccache libffi-dev libssl-dev

# Crear directorio de trabajo
mkdir -p ~/esp
cd ~/esp
```

#### 2. Instalar ESP-IDF
```bash
# Clonar repositorio ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# Ejecutar instalador
./install.sh esp32c6

# Configurar variables de entorno
source ./export.sh
```

#### 3. Clonar proyecto del Nodo de Cisterna
```bash
cd ~/esp
git clone <URL_DEL_REPOSITORIO> nodo_cisterna
cd nodo_cisterna
```

#### 4. Configurar el Proyecto
```bash
# Copiar configuración base
idf.py set-target esp32c6

# Abrir menú de configuración (OPCIONAL)
# Cambiar SSID, contraseña MQTT, pines GPIO según sea necesario
idf.py menuconfig
```

#### 5. Compilar el Firmware
```bash
# Compilar proyecto
idf.py build

# Salida esperada: nodo_cisterna/build/nodo_cisterna.bin
```

#### 6. Cargar Firmware en ESP32-C6
```bash
# Conectar ESP32-C6 por USB
# Identificar puerto serial: /dev/ttyUSB0 o /dev/ttyACM0

# Cargar firmware
idf.py -p /dev/ttyUSB0 flash

# Monitorear salida (Ctrl+] para salir)
idf.py -p /dev/ttyUSB0 monitor
```

---

## Configuración

### Archivo `sdkconfig`
Contiene configuraciones base. Se puede generar con `idf.py menuconfig`

### Credenciales Wi-Fi y MQTT
Editar en `main/main.c`:
```c
esp_err_t wifi_err = wifi_init("TU_SSID_AQUI", "TU_PASSWORD_AQUI");

mqtt_config_t mqtt_cfg = {
    .broker_uri = "mqtt://192.168.1.100:1883",
    .client_id = "esp32c6_cisterna",
    ...
};
```

### Pines GPIO
Editar en `main/main.c` antes de `tasks_init()`:
```c
task_config_t task_cfg = {
    .sampling_interval_ms = 1000,
    .ultrasonic_trig_pin = GPIO_NUM_10,
    .ultrasonic_echo_pin = GPIO_NUM_9,
    .tds_adc_pin = 0,
    .pump_relay_pin = GPIO_NUM_8
};
```

---

## Pruebas y Debugging

### 1. Verificar Conexión Wi-Fi
```
[WIFI] → Iniciando conexión a Wi-Fi...
[WIFI] ✓ Conectado a Wi-Fi | IP obtenida: 192.168.1.50
```

### 2. Verificar Conexión MQTT
```
[MQTT] ✓ Cliente MQTT inicializado
[MQTT] → Iniciando conexión a broker MQTT...
[MQTT] ✓ MQTT conectado al broker
```

### 3. Verificar Lectura de Sensores
```
[TASKS] → Tarea de lectura de sensores iniciada
[SENSOR] ✓ Sensores inicializados correctamente
Lectura #1 | Nivel: 125.50 cm | TDS: 450.2 ppm (MEDIA) | Bomba: ON
```

### 4. Publicación MQTT
```bash
# En otra terminal, suscribirse a mensajes
mosquitto_sub -h 192.168.1.100 -t "cistern_sensordata"

# Salida esperada:
{"nivel_agua_cm":125.50,"tds_ppm":450.2,"estado_agua":"MEDIA","estado_bomba":"ON","timestamp":1234}
```

### 5. Control Manual de Bomba
```bash
# Encender bomba
mosquitto_pub -h 192.168.1.100 -t "cistern_control" -m "ON"

# Apagar bomba
mosquitto_pub -h 192.168.1.100 -t "cistern_control" -m "OFF"

# Activar automático
mosquitto_pub -h 192.168.1.100 -t "cistern_control" -m "AUTO"
```

### 6. Calibración TDS (UART)

- **Descripción:** El firmware incluye comandos sencillos accesibles desde el monitor serie (UART) para calibrar el sensor TDS en campo.

- **Comandos disponibles:**
  - `calA` : Captura la lectura ADC actual y la guarda como punto A (offset).
  - `calB` : Captura la lectura ADC actual y calcula la ganancia (gain) relativa al punto A.
  - `save` : Persiste `tds_offset` y `tds_gain` en NVS (memoria no volátil).
  - `show` : Muestra los valores actuales de `tds_offset` y `tds_gain`.

- **Cómo usar:**
  1. Abrir el monitor serie: `idf.py -p /dev/ttyUSB0 monitor`
  2. Escribir el comando (`calA`, `calB`, `save`, `show`) y pulsar Enter (asegurar CR/LF).

- **Flujo recomendado de calibración:**
  1. Colocar el sensor en una muestra de referencia A (p. ej. agua destilada).
  2. `calA` → toma lectura raw y la guarda como `offset`.
  3. Colocar el sensor en una segunda muestra de referencia B (p. ej. solución con ppm conocida).
  4. `calB` → toma lectura raw y calcula `gain = 1.0f / (rawB - offset)` (el firmware evita división por cero).
  5. `save` → persiste `offset` y `gain` en NVS.
  6. `show` → verificar valores guardados.

- **Fórmula usada por el firmware:**

  tds_ppm = (raw - offset) * gain * 1000.0f

  - `raw` = lectura ADC actual
  - `offset` = valor obtenido con `calA`
  - `gain` = factor calculado con `calB`

- **Notas y recomendaciones:**
  - Calibrar con el sensor en condiciones estables y con soluciones de referencia conocidas.
  - Ejecutar `calA` y `calB` en ese orden antes de `save`.
  - Tras guardar, los valores se cargan automáticamente al iniciar el dispositivo.
  - Por estabilidad del sistema, el proyecto reemplazó el uso de `esp_console`/linenoise por un lector UART mínimo que procesa líneas simples; esto evita problemas de inestabilidad relacionados con `vfprintf` o la pila.

- **Ejemplo de sesión (monitor serie):**

```
[SENSOR] ✓ Punto A calibrado: offset=1234
[SENSOR] ✓ Punto B calibrado: gain=0.002345
[SENSOR] ✓ Calibración guardada: offset=1234 gain=0.002345
[SENSOR] Calibración actual: offset=1234 gain=0.002345
```

---

## Troubleshooting

### Problema: No conecta a Wi-Fi
- Verificar SSID y contraseña en `main.c`
- Asegurar que ESP32-C6 está dentro del rango de la red
- Revisar logs: `idf.py monitor`

### Problema: MQTT no conecta
- Verificar que broker está en funcionamiento
- Verificar dirección IP y puerto del broker
- Asegurar que red permite tráfico MQTT (puerto 1883)

### Problema: Sensor ultrasónico no lee
- Verificar pines TRIG y ECHO correctos
- Asegurar cable sin roturas
- Probar con multímetro si cables están conectados

### Problema: Sensor TDS da valores raros
- Verificar calibración del sensor
- Revisar rango de voltaje ADC
- Limpiar sensor si está obstruido

### Problema: Compilación falla
```bash
# Limpiar build
idf.py fullclean

# Reconstruir
idf.py build

# Si persiste, verificar versión ESP-IDF
idf.py --version
```

---

## Especificaciones Técnicas

### Sensor Ultrasónico HC-SR04
- **Rango de medición:** 2 cm - 4 m
- **Resolución:** ~0.3 cm
- **Frecuencia de operación:** 40 kHz
- **Voltaje:** 5V (compatibilidad con ESP32 con divisor de voltaje)

### Sensor TDS
- **Rango de medición:** 0 - 1000 ppm (configurable)
- **Resolución:** ~1 ppm
- **Voltaje:** 3.3V (compatible con ADC de ESP32)

### Relé HW-307
- **Voltaje de activación:** 5V
- **Corriente máxima:** 30A
- **Tipo de contactos:** NO/NC

### ESP32-C6
- **Processor:** Xtensa 32-bit @ 160 MHz
- **RAM:** 512 KB SRAM
- **Flash:** 4 MB (típicamente)
- **GPIO:** 30 pines
- **ADC:** 2x ADC de 12 bits

---

## Notas de Desarrollo

### Sincronización de Datos
Se utiliza un **semáforo mutex** para evitar condiciones de carrera entre:
- Tarea de lectura de sensores (escribe datos)
- Tarea principal (lee datos)

### Tareas FreeRTOS
```
Prioridad 3: sensor_read_and_publish_task (más alta)
Prioridad 2: sensor_read_task (lectura periódica)
Prioridad 0: vTaskDelay en main (baja)
```

### Manejo de Errores
Todas las funciones retornan `esp_err_t`. Revisar logs de ESP_LOGE/ESP_LOGW para debugging.

---

## Licencia y Autoría

Proyecto de Sistema IoT para Control de Cisterna
Desarrollado para Universidad Nacional de Colombia
Diciembre 2025

---

## Referencias y Recursos

- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
- [ESP32-C6 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf)
- [MQTT Protocol Specification](https://mqtt.org/mqtt-specification)
- [FreeRTOS Reference](https://www.freertos.org/Documentation/RTOS_Getting_Started.html)

---

**Última actualización:** Diciembre 7, 2025
