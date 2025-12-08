# Guía de Configuración del Nodo de Cisterna

## 📋 Tabla de Contenidos
1. [Configuración Inicial](#configuración-inicial)
2. [Pines GPIO](#pines-gpio)
3. [Parámetros de Control](#parámetros-de-control)
4. [Credenciales de Red](#credenciales-de-red)
5. [Calibración de Sensores](#calibración-de-sensores)

---

## Configuración Inicial

### Modificar Credenciales Wi-Fi

**Archivo:** `main/main.c` (línea ~200)

```c
// ANTES
esp_err_t wifi_err = wifi_init("TU_SSID_AQUI", "TU_PASSWORD_AQUI");

// DESPUÉS (Ejemplo)
esp_err_t wifi_err = wifi_init("MiRedWiFi", "contraseña123");
```

### Modificar Configuración MQTT

**Archivo:** `main/main.c` (línea ~215)

```c
mqtt_config_t mqtt_cfg = {
    .broker_uri = "mqtt://192.168.1.100:1883",  // IP de tu broker
    .client_id = "esp32c6_cisterna",
    .username = "",      // Dejar vacío si no hay autenticación
    .password = ""       // Dejar vacío si no hay autenticación
};
```

---

## Pines GPIO

### Asignación Actual
```
GPIO 10 ← Sensor Ultrasónico TRIG
GPIO 9  ← Sensor Ultrasónico ECHO
ADC 0   ← Sensor TDS
GPIO 8  ← Relé Bomba HW-307
```

### Modificar Pines

**Archivo:** `main/main.c` (línea ~230)

```c
task_config_t task_cfg = {
    .sampling_interval_ms = 1000,
    .ultrasonic_trig_pin = GPIO_NUM_10,   // Cambiar aquí
    .ultrasonic_echo_pin = GPIO_NUM_9,    // Cambiar aquí
    .tds_adc_pin = 0,                     // Cambiar aquí
    .pump_relay_pin = GPIO_NUM_8          // Cambiar aquí
};
```

**Importante:** Los pines disponibles en ESP32-C6 son 0-20 y 22-30 (ver datasheet)

---

## Parámetros de Control

### Umbrales de Nivel de Agua

**Ubicación:** `main/main.c` en función `sensor_read_and_publish_task()`

```c
// Valores actuales:
bool level_low = (sensor_data.water_level < 20.0f);   // 20 cm
bool level_high = (sensor_data.water_level > 180.0f);  // 180 cm
```

**Cambiar según capacidad de cisterna:**

Ejemplo para cisterna de 1m x 1m:
```c
bool level_low = (sensor_data.water_level < 30.0f);    // 30 cm
bool level_high = (sensor_data.water_level > 150.0f);  // 150 cm
```

### Intervalos de Muestreo

**Ubicación:** `main/main.c` en `app_main()`

```c
.sampling_interval_ms = 1000,  // 1 segundo (cambiar a preferencia)
```

Valores recomendados:
- `500` ms = Lectura cada medio segundo (más actualizaciones, más consumo)
- `1000` ms = Lectura cada segundo (estándar)
- `5000` ms = Lectura cada 5 segundos (menos consumo)

---

## Credenciales de Red

### Si el Broker MQTT está en Raspberry Pi

```c
mqtt_config_t mqtt_cfg = {
    .broker_uri = "mqtt://raspberrypi.local:1883",
    // O usar IP directa
    .broker_uri = "mqtt://192.168.1.150:1883",
};
```

### Con Autenticación MQTT

```c
mqtt_config_t mqtt_cfg = {
    .broker_uri = "mqtt://192.168.1.100:1883",
    .client_id = "esp32c6_cisterna",
    .username = "usuario_mqtt",
    .password = "contraseña_mqtt"
};
```

### Con Conexión TLS/Segura

```c
.broker_uri = "mqtts://broker.example.com:8883",
```

---

## Calibración de Sensores

### Calibración del Sensor Ultrasónico

1. **Medida conocida:** Coloca objeto a 20 cm del sensor
2. **Leer valor:** Observa logs en `idf.py monitor`
3. **Ajustar:** Si mide 25 cm en lugar de 20 cm, hay desfase de +5 cm

En `components/sensors/sensor.c`:
```c
// Línea ~135 - Ajuste de distancia
*distance = (echo_duration * 0.0343f) / 2.0f;

// Si hay desfase, restar/sumar:
*distance = ((echo_duration * 0.0343f) / 2.0f) - 0.5f;  // Restar 0.5 cm
```

### Calibración del Sensor TDS

1. **Solución de referencia:** Usar agua destilada (~0 ppm) y agua salina (~1000 ppm)
2. **Registrar voltajes:** Anotar valores ADC para cada solución
3. **Recalibrar fórmula:**

En `components/sensors/sensor.c`, línea ~165:
```c
// Fórmula actual (genérica)
*tds_value = (voltage - 0.05f) / 0.065f;

// Recalibración personalizada:
// Si tienes valores de referencia (V1,ppm1) y (V2,ppm2):
// *tds_value = (voltage - V1) * (ppm2 - ppm1) / (V2 - V1) + ppm1
```

### Procedimiento de Calibración Completo

```bash
# 1. Conectar ESP32-C6
# 2. Monitorear salida
idf.py -p /dev/ttyUSB0 monitor

# 3. Observar lecturas crudas
# 4. Sumergir sensores en soluciones conocidas
# 5. Anotar valores
# 6. Ajustar constantes en código
# 7. Recompilar y cargar
idf.py build
idf.py -p /dev/ttyUSB0 flash
```

---

## Configuración Avanzada

### Modificar Estados de Agua

**Archivo:** `components/sensors/sensor.h` (línea ~44)

```c
#define WATER_CLEAN_THRESHOLD 300    // < 300 ppm
#define WATER_MEDIUM_THRESHOLD 600   // 300-600 ppm
```

### Cambiar Lógica de Control de Bomba

**Archivo:** `main/main.c` en `sensor_read_and_publish_task()`

Lógica actual:
```c
if (level_low && water_acceptable) {
    tasks_set_pump_relay(true);    // Encender
} else if (level_high || !water_acceptable) {
    tasks_set_pump_relay(false);   // Apagar
}
```

Ejemplo: Solo encender si nivel muy bajo Y agua limpia:
```c
if ((level_low) && (sensor_data.water_state == WATER_STATE_CLEAN)) {
    tasks_set_pump_relay(true);
} else if (level_high || sensor_data.water_state != WATER_STATE_CLEAN) {
    tasks_set_pump_relay(false);
}
```

---

## Verificación de Configuración

Después de realizar cambios:

```bash
# 1. Compilar
idf.py build

# 2. Verificar errores (debe haber 0 errores)
idf.py build 2>&1 | grep -i "error" | wc -l

# 3. Cargar
idf.py -p /dev/ttyUSB0 flash

# 4. Monitorear
idf.py -p /dev/ttyUSB0 monitor

# 5. Buscar "✓" (checkmarks) para confirmar inicialización
```

---

## Troubleshooting de Configuración

### Error: "SSID too long"
SSID máximo 32 caracteres. Reducir nombre de red.

### Error: "Password too short"
Contraseña mínimo 8 caracteres (WPA2).

### Error: "Unknown GPIO number"
Verificar números de pin contra datasheet ESP32-C6.

### Sensor no inicializa
Revisar que los pines no están en conflicto con GPIO de arranque (0, 2, 12, etc.)

---

**Última actualización:** Diciembre 2025
