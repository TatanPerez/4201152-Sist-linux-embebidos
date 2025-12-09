# Node Tank - Sistema de Monitoreo de Nivel de Agua

Nodo IoT basado en ESP32-C6 que monitorea el nivel de agua de un tanque mediante un sensor ultrasónico y transmite los datos a través de MQTT.

## 📋 Descripción del Proyecto

Node Tank es un sistema embebido para monitoreo remoto del nivel de agua en tanques. El sistema se conecta a una red Wi-Fi, lee el nivel del agua usando un sensor ultrasónico y publica los datos en un broker MQTT para su visualización y análisis en tiempo real.

**Características principales:**
- ✅ Conectividad Wi-Fi automática (Modo STA)
- ✅ Comunicación MQTT para IoT
- ✅ Lectura de sensor ultrasónico HC-SR04
- ✅ Tareas concurrentes con FreeRTOS
- ✅ Sincronización de acceso a recursos compartidos
- ✅ Almacenamiento no volátil (NVS) para configuración

## 🔧 Requisitos Previos

### Hardware
- **Placa:** ESP32-C6
- **Sensor:** HC-SR04 (sensor ultrasónico de distancia)
- **Conexión:** Red Wi-Fi (2.4 GHz)
- **Broker MQTT:** Servidor MQTT accesible en la red

### Software
- **ESP-IDF:** Versión 5.0 o superior
- **CMake:** Versión 3.5 o superior
- **Python:** 3.7 o superior
- **Git:** Para control de versiones

### Dependencias del Proyecto
- FreeRTOS (incluido en ESP-IDF)
- ESP-MQTT (cliente MQTT de Espressif)
- NVS Flash (almacenamiento de configuración)
- Wi-Fi Stack (ESP-IDF)

## 📁 Estructura del Proyecto

```
Node_Tank/
├── CMakeLists.txt           # Configuración de compilación CMake
├── sdkconfig                # Configuración del SDK
├── build/                   # Directorio de compilación (generado)
├── main/                    # Código principal de la aplicación
│   ├── CMakeLists.txt
│   ├── main.c              # Punto de entrada, inicialización
│   ├── wifi.c / wifi.h     # Módulo de conectividad Wi-Fi
│   ├── mqtt.c / mqtt.h     # Cliente MQTT
│   ├── sensor.c / sensor.h # Interfaz del sensor ultrasónico
│   ├── sensor_ultrasonico.c / .h  # Implementación específica
│   └── tasks.c / tasks.h   # Tareas FreeRTOS
└── esp-idf/                 # SDK de Espressif (incluido)
```

## 🚀 Instalación y Configuración

### 1. Clonar el Repositorio

```bash
git clone https://github.com/TatanPerez/4201152-Sist-linux-embebidos.git
cd Proyecto/Node_Tank
```

### 2. Configurar ESP-IDF

Si no tienes ESP-IDF instalado:

```bash
# Descargar ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
source export.sh
```

### 3. Configurar Credenciales Wi-Fi y MQTT

Editar `main/main.c` y actualizar las siguientes líneas:

```c
#define DEFAULT_WIFI_SSID "Tu_Red_WiFi"
#define DEFAULT_WIFI_PASS "Tu_Contraseña"
#define DEFAULT_MQTT_BROKER_URI "mqtt://IP_BROKER:1883"
```

Alternativamente, usar `menuconfig`:

```bash
idf.py menuconfig
# Navegar a: Main application configuration
```

### 4. Compilar el Proyecto

```bash
idf.py build
```

### 5. Flashear en el ESP32-C6

```bash
# Detectar puerto serial automáticamente
idf.py flash

# O especificar el puerto manualmente
idf.py -p /dev/ttyUSB0 flash
```

### 6. Monitorear Logs

```bash
idf.py monitor

# O especificar el puerto
idf.py -p /dev/ttyUSB0 monitor
```

## 📡 Módulos del Proyecto

### 1. **WiFi Module** (`wifi.h / wifi.c`)
Gestiona la conexión a la red Wi-Fi.

```c
void wifi_init_sta(const char *ssid, const char *password);
```
- Inicializa la interfaz de red (esp-netif)
- Conecta en modo STA (Station)
- Maneja eventos de conexión/desconexión

### 2. **MQTT Module** (`mqtt.h / mqtt.c`)
Cliente MQTT para publicación de datos.

```c
esp_err_t mqtt_init(const char *broker_uri);
esp_err_t mqtt_publish(const char *topic, const char *payload);
```
- Conecta con broker MQTT
- Publica mensajes con QoS=1
- Maneja reconexiones automáticas

### 3. **Sensor Module** (`sensor.h / sensor.c`)
Interfaz abstrata del sensor.

```c
void sensor_init(void);
float sensor_read_level_cm(void);
```
- Inicializa recursos (pines, timers)
- Retorna el nivel en centímetros
- Compatible con HC-SR04 y otros sensores

### 4. **Tasks Module** (`tasks.h / tasks.c`)
Orquestación de tareas concurrentes con FreeRTOS.

```c
void tasks_start(SemaphoreHandle_t mutex);
```
- Crea tareas de lectura del sensor
- Sincroniza acceso a recursos compartidos
- Publica datos cada 5 segundos

## 🔌 Configuración del Hardware

### Conexiones del Sensor Ultrasónico HC-SR04

| Componente | Pata | GPIO ESP32-C6 |
|------------|------|---------------|
| HC-SR04    | VCC  | 3.3V          |
| HC-SR04    | GND  | GND           |
| HC-SR04    | TRIG | GPIO 4        |
| HC-SR04    | ECHO | GPIO 5        |

*Nota: Los pines pueden modificarse en `sensor_ultrasonico.c`*

## 📊 Flujo de Datos

```
┌─────────────────┐
│   Sensor HC-SR04 │
│  (Nivel agua)    │
└────────┬─────────┘
         │
         ▼
┌─────────────────────────┐
│  FreeRTOS Task          │
│  Lee sensor cada 5s     │
└────────┬────────────────┘
         │
         ▼
┌─────────────────────────┐
│  MQTT Client            │
│  Publica en topic       │
└────────┬────────────────┘
         │
         ▼
┌─────────────────────────┐
│  MQTT Broker            │
│  (Servidor externo)     │
└─────────────────────────┘
```

## 📨 Tópicos MQTT

- **Publicación:** `sensor/tank/level` - Nivel del agua en centímetros
  - Payload: `{"level": 45.2, "timestamp": "2025-12-09T10:30:45Z"}`

## 🔐 Seguridad

- **TLS/SSL:** Actualmente sin cifrado (usar `mqtts://` para producción)
- **Autenticación MQTT:** No configurada por defecto
- **Credenciales:** Almacenar en NVS Flash en lugar de hardcoding

## 🐛 Solución de Problemas

### El dispositivo no se conecta a Wi-Fi
- Verificar SSID y contraseña en `main.c`
- Asegurar que la red opera en 2.4 GHz
- Revisar logs: `idf.py monitor`

### No hay datos en el broker MQTT
- Verificar IP y puerto del broker con: `ping IP_BROKER`
- Usar cliente MQTT para validar: `mosquitto_sub -h IP_BROKER -t sensor/tank/level`
- Revisar conexión Wi-Fi

### Sensor retorna valores incorrectos
- Verificar conexiones de TRIG y ECHO
- Asegurar que el sensor está a más de 2 cm del objeto
- Probar con objeto estacionario a distancia conocida

## 📈 Mejoras Futuras

- [ ] Interfaz web para configuración remota
- [ ] Almacenamiento de histórico en SD card
- [ ] Alertas cuando nivel está fuera de rango
- [ ] Encriptación TLS para MQTT
- [ ] Bajo consumo de energía (modo deep sleep)
- [ ] Múltiples sensores en cascada

## 📚 Referencias

- [Documentación ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [ESP32-C6 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf)
- [HC-SR04 Ultrasonic Sensor](https://www.electroschematics.com/hc-sr04-ultrasonic-sensor/)
- [MQTT Protocol](https://mqtt.org/)

## 📄 Licencia

Proyecto académico - Universidad Nacional de Colombia (UNAL)

## 👨‍💻 Autor

**Tatán Pérez**  
Licencia Ingeniería de Sistemas (8vo semestre)  
Sistemas Linux Embebidos - 4201152

---

**Última actualización:** Diciembre 2025
