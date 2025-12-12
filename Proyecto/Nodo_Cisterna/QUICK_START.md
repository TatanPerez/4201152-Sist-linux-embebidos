# 🚀 GUÍA RÁPIDA DE INICIO - Nodo Cisterna ESP32-C6

## ⚡ Pasos Rápidos (5 minutos)

### 1️⃣ En Windows (PowerShell)

```powershell
# Abre PowerShell en el directorio del proyecto
cd "c:\Users\tatan\Documentos\UNAL\8_License_Plate\Linux\Proyecto\Node Cisterna\nodo_cisterna"

# Abre WSL Ubuntu desde aquí
wsl

# Verás el símbolo del sistema de Linux:
```

### 2️⃣ En WSL Ubuntu

```bash
# 1. Navega al proyecto
cd /mnt/c/Users/tatan/Documentos/UNAL/8_License_Plate/Linux/Proyecto/Node\ Cisterna/nodo_cisterna

# 2. Configura variables de entorno
source ~/esp/esp-idf/export.sh

# 3. EDITA las credenciales (sustituye valores)
nano main/main.c
# Busca línea ~200: wifi_init("TU_SSID_AQUI", "TU_PASSWORD_AQUI")
# Presiona: Ctrl+X, luego Y, luego Enter

# 4. Compila el proyecto
idf.py build

# 5. Conecta ESP32-C6 por USB

# 6. Carga el firmware
idf.py -p /dev/ttyUSB0 flash

# 7. Monitorea la salida
idf.py -p /dev/ttyUSB0 monitor
# (Presiona Ctrl+] para salir)
```

---

## ✅ Verificación Rápida

Debes ver en los logs:

```
✓ NVS Flash inicializado
✓ Wi-Fi inicializado y conectado
✓ MQTT conectado al broker
✓ Sensores inicializados
✓ Tarea de lectura iniciada

Lectura #1 | Nivel: 125.50 cm | TDS: 450.2 ppm (MEDIA) | Bomba: OFF
```

---

## 🔧 Configuración Básica Necesaria

### Cambiar SSID y Contraseña Wi-Fi

**Archivo:** `main/main.c` (línea ~200)

```c
// ANTES
esp_err_t wifi_err = wifi_init("TU_SSID_AQUI", "TU_PASSWORD_AQUI");

// DESPUÉS - Ejemplo para red "CasaWifi" con contraseña "12345678"
esp_err_t wifi_err = wifi_init("CasaWifi", "12345678");
```

### Cambiar IP del Broker MQTT

**Archivo:** `main/main.c` (línea ~215)

```c
mqtt_config_t mqtt_cfg = {
    .broker_uri = "mqtt://192.168.1.100:1883",  // ← Cambiar esta IP
    .client_id = "esp32c6_cisterna",
};
```

**Encontrar IP del Broker:**

En Linux/Windows:
```bash
# Si el broker está en Raspberry Pi
ping raspberrypi.local

# O usar IP directa
ping 192.168.1.150
```

---

## 📱 Probar Sistema con MQTT

### Instalar Cliente MQTT (Ubuntu)

```bash
sudo apt install -y mosquitto-clients
```

### Suscribirse a Mensajes del Sensor

```bash
# En terminal 1
mosquitto_sub -h 192.168.1.100 -t "cistern_sensordata"

# Verás mensajes cada segundo:
{"nivel_agua_cm":125.50,"tds_ppm":450.2,"estado_agua":"MEDIA","estado_bomba":"OFF","timestamp":1234}
```

### Controlar Bomba Manualmente

```bash
# En terminal 2
mosquitto_pub -h 192.168.1.100 -t "cistern_control" -m "ON"  # Encender
mosquitto_pub -h 192.168.1.100 -t "cistern_control" -m "OFF" # Apagar
mosquitto_pub -h 192.168.1.100 -t "cistern_control" -m "ON"  # Encender la bomba desde Node-RED
mosquitto_pub -h 192.168.1.100 -t "cistern_control" -m "OFF" # Apagar la bomba desde Node-RED

Nota: El control manual tradicional (MANUAL) se sustituye por un botón físico conectado al ESP32: al pulsar, la bomba se enciende; al soltar, se apaga.
```

---

## 🐛 Si Algo Falla

### Error: "Puerto /dev/ttyUSB0 no encontrado"

```bash
# Listar puertos disponibles
ls /dev/tty*

# Cambiar comando a puerto correcto (ej: /dev/ttyACM0)
idf.py -p /dev/ttyACM0 flash
idf.py -p /dev/ttyACM0 monitor
```

### Error: "Connection refused" (MQTT)

```bash
# Verificar que broker está corriendo
mosquitto_pub -h 192.168.1.100 -t "test" -m "hello"

# O iniciar Mosquitto en Raspberry Pi
sudo systemctl start mosquitto
```

### Compilación falla

```bash
# Limpiar build y recompilar
idf.py fullclean
idf.py build
```

---

## 📡 Arquitectura MQTT

```
ESP32-C6 Node
     ↓
     └──→ WiFi (2.4 GHz)
            ↓
         Router
            ↓
    Raspberry Pi / Broker
            ↓
         MQTT Topics:
         ├── cistern_sensordata  (publicar cada 1s)
         └── cistern_control     (recibir comandos)
```

---

## 📝 Estructura de Archivos

```
nodo_cisterna/
├── main/
│   └── main.c          ← EDITAR: credenciales Wi-Fi/MQTT
├── components/
│   ├── wifi/           ← Manejo de conectividad
│   ├── mqtt/           ← Comunicación MQTT
│   ├── sensors/        ← Lectura de sensores
│   └── tasks/          ← Tareas FreeRTOS
├── README.md           ← Documentación completa
├── CONFIGURACION.md    ← Parámetros avanzados
└── ESQUEMA_CONEXIONES.md ← Diagrama de pines
```

---

## 🔌 Pines Utilizados

```
GPIO-10 → Sensor Ultrasónico TRIG
GPIO-9  → Sensor Ultrasónico ECHO
ADC-0   → Sensor TDS
GPIO-8  → Relé Control Bomba
GND     → Tierra común
3.3V    → Alimentación sensores
5V      → Alimentación relé
```

**Ver ESQUEMA_CONEXIONES.md para diagrama detallado**

---

## 💡 Comandos Útiles

```bash
# Ver versión ESP-IDF
idf.py --version

# Monitorear salida (con filtro)
idf.py -p /dev/ttyUSB0 monitor | grep "✓"

# Cambiar configuración GUI
idf.py menuconfig

# Compilar solamente (sin cargar)
idf.py build

# Cargar solo (sin compilar)
idf.py -p /dev/ttyUSB0 flash

# Borrar datos del ESP
idf.py -p /dev/ttyUSB0 erase-flash
```

---

## 📊 Valores Esperados

| Métrica | Rango | Notas |
|---------|-------|-------|
| Nivel agua | 0-250 cm | Depende de altura cisterna |
| TDS | 0-1000 ppm | Conductividad del agua |
| WiFi | 2.4 GHz | Compatible con ESP32-C6 |
| MQTT | QoS 1 | Garantizado una vez |
| Muestreo | 1000 ms | Configurable en main.c |

---

## ⏱️ Línea de Tiempo Típica

```
0s   → Encender ESP32-C6
2s   → Inicializar Flash
3s   → Conectar WiFi
7s   → Conectar MQTT
10s  → Inicializar Sensores
11s  → Empezar a publicar datos

Esperado: ~10 segundos desde encendido hasta datos activos
```

---

## 🎯 Próximos Pasos

### Después de Verificar Funcionamiento Básico

1. **Calibrar Sensores** (CONFIGURACION.md)
2. **Ajustar Umbrales de Bomba** (main.c)
3. **Configurar Alertas** (app adicional)
4. **Dashboard Web** (Node-RED, Home Assistant, etc.)

---

## 📞 Soporte Rápido

| Problema | Solución |
|----------|----------|
| No conecta WiFi | Verificar SSID/password en main.c |
| No conecta MQTT | Verificar IP y puerto del broker |
| Sensor no lee | Revisar pines GPIO en ESQUEMA_CONEXIONES.md |
| Compilación falla | Ejecutar `idf.py fullclean` |
| Puerto no encontrado | Ver "Error: Puerto /dev/ttyUSB0" arriba |

---

## 📚 Documentación Completa

- **README.md** - Guía completa del proyecto
- **CONFIGURACION.md** - Parámetros y calibración
- **ESQUEMA_CONEXIONES.md** - Diagrama de pines y conexiones
- **main.c** - Comentarios detallados del código

---

**⏱️ Tiempo total de instalación: 30-45 minutos**

**✅ El sistema estará listo para monitorear y controlar tu cisterna**

---

**Última actualización:** Diciembre 2025
**Versión:** 1.0 Quick Start
