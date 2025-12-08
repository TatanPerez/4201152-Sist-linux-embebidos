# Diagnóstico y Configuración del Proyecto ESP32-C6 — Cisterna

## Resumen de lo que pasaba

La salida que enviaste mostró dos problemas:

1. **Error de Wi-Fi: `ESP_ERR_INVALID_STATE`**  
   - Causa: El loop de eventos y netif de ESP-IDF no estaban inicializados antes de registrar handlers.
   - **Solución**: Agregué `esp_netif_init()` y `esp_event_loop_create_default()` en `components/wifi/wifi.c` (línea ~76-87).

2. **Sensor ultrasónico con "Timeout esperando ECHO alto"**  
   - Causa: El timeout de 1ms era muy corto; sensores típicos tardaban más.
   - **Solución**: Aumenté el timeout a 30ms en `components/sensors/sensor.c` (línea ~118).

3. **MQTT stub (sin cliente real)**  
   - Es intencional. El código funciona offline sin él. Cuando tengas el broker MQTT real, lo reemplazaremos.

---

## Cambios realizados

### 1. `components/wifi/wifi.c` (línea ~76-87)
```c
// Agregar ANTES de registrar handlers:
esp_err_t ret = esp_netif_init();
if (ret != ESP_OK && ret != ESP_ERR_NO_MEM && ret != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "✗ Error inicializando esp_netif: %s", esp_err_to_name(ret));
    return ret;
}

ret = esp_event_loop_create_default();
if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "✗ Error creando event loop por defecto: %s", esp_err_to_name(ret));
    return ret;
}
```

### 2. `components/sensors/sensor.c` (línea ~118)
```c
// Cambiar de:
uint32_t timeout = 1000;  // 1ms timeout
// A:
uint32_t timeout = 30000;  // 30ms timeout
```

---

## Configurar credenciales Wi-Fi y MQTT

### Wi-Fi
**Archivo**: `main/main.c` (línea ~167)

Busca:
```c
esp_err_t wifi_err = wifi_init("TU_SSID_AQUI", "TU_PASSWORD_AQUI");
```

Reemplaza con:
```c
esp_err_t wifi_err = wifi_init("Tu_Red_WiFi", "TuContrasena123");
```

### MQTT
**Archivo**: `main/main.c` (línea ~191-197)

Busca:
```c
mqtt_config_t mqtt_cfg = {
    .broker_uri = "mqtt://10.162.31.132:1883",
    .client_id = "esp32c6_cisterna",
    .username = "",
    .password = ""
};
```

Reemplaza:
- `.broker_uri`: URL de tu broker (ej: `mqtt://192.168.1.100:1883`)
- `.client_id`: identificador único del chip
- `.username` / `.password`: solo si tu broker lo requiere

---

## Pines usados

Los pines están configurados en `main/main.c` (línea ~208-213):

```c
task_config_t task_cfg = {
    .sampling_interval_ms = 1000,        // 1 segundo
    .ultrasonic_trig_pin = GPIO_NUM_5,   // TRIG sensor ultrasónico
    .ultrasonic_echo_pin = GPIO_NUM_18,  // ECHO sensor ultrasónico
    .tds_adc_pin = 0,                    // Canal ADC 0 (sensor TDS)
    .pump_relay_pin = GPIO_NUM_8         // Pin relé bomba
};
```

### Mapeo de pines en ESP32-C6
- **GPIO_NUM_5** → TRIG (salida digital, pulso de 10µs)
- **GPIO_NUM_18** → ECHO (entrada digital, mide duración del pulso)
- **ADC1_CHANNEL_0** → Sensor TDS (entrada analógica 0-3.3V)
- **GPIO_NUM_8** → Relé (salida digital, 3.3V/driver)

### Cableado recomendado

**Sensor Ultrasónico (HC-SR04 compatible)**:
- VCC → 5V (o según sensor)
- GND → GND
- TRIG → GPIO_NUM_5
- ECHO → GPIO_NUM_18 (con divisor de tensión si sensor es 5V: usar 1kΩ + 2kΩ para bajar a 3.3V)

**Sensor TDS (Analógico)**:
- VCC → 3.3V
- GND → GND
- SIG → ADC1_CHANNEL_0 (GPIO1 en ESP32-C6, típicamente)

**Relé Bomba (HW-307 u similar)**:
- VCC → 5V (potencia del relé)
- GND → GND
- IN → GPIO_NUM_8
- Conexión de potencia de la bomba en los contactos del relé

---

## Próximos pasos

### Flashear el código
```bash
cd /mnt/c/Users/tatan/Documentos/UNAL/8_License_Plate/Linux/Proyecto/Node\ Cisterna/nodo_cisterna

# Si IDF no está exportado:
export IDF_PATH=~/esp/esp-idf
source $IDF_PATH/export.sh

# Flashear (ajusta /dev/ttyUSB0 a tu puerto):
idf.py -p /dev/ttyUSB0 flash

# Monitor (ver logs en vivo):
idf.py -p /dev/ttyUSB0 monitor
```

### Qué esperar al flashear (con las correcciones)
1. Wi-Fi debería intentar conectar sin error `ESP_ERR_INVALID_STATE`.
2. Sensor ultrasónico debería leer correctamente (si está bien conectado).
3. Relé debería encender/apagar según el nivel de agua.
4. MQTT seguirá siendo stub hasta que instales el cliente real.

### Usar MQTT real (opcional, pasos futuros)
1. Ejecutar: `idf.py add-dependency esp-mqtt` (o instalar desde component manager).
2. Restaurar `components/mqtt/mqtt.c` y `mqtt.h` para usar `mqtt_client.h`.
3. Recompilar con `idf.py build`.

---

## Checklist de verificación

- [ ] Edité credenciales Wi-Fi en `main/main.c` línea ~167
- [ ] Edité config MQTT en `main/main.c` línea ~191-197
- [ ] Verifiqué conexiones de pines (TRIG, ECHO, TDS, relé)
- [ ] Compilé con `idf.py build` (ya hecho, archivos listos)
- [ ] Flasheé con `idf.py -p (PORT) flash`
- [ ] Abrí monitor con `idf.py -p (PORT) monitor`
- [ ] Verifiqué logs: Wi-Fi conectando, sensores leyendo, relé respondiendo

---

## Preguntas frecuentes

**P: ¿Por qué MQTT sigue siendo stub?**  
R: Es intencional. El cliente oficial requiere instalar un componente adicional. Cuando lo hagas, reemplazamos mqtt.c/mqtt.h.

**P: El sensor ultrasónico sigue sin leer.**  
R: Verifica:
1. Pines correctos (GPIO_NUM_5 = TRIG, GPIO_NUM_18 = ECHO)
2. Voltaje: si sensor es 5V, ECHO necesita divisor de tensión
3. Cableado: ¿está bien conectado GND?

**P: Wi-Fi no conecta.**  
R: Verifica:
1. SSID y contraseña correctos en `main/main.c`
2. Red en 2.4GHz (ESP32-C6 no soporta 5GHz)
3. Monitor/logs para ver el error exacto

**P: ¿Cómo integro MQTT real?**  
R: Cuando estés listo, dímelo. Puedo hacerlo en 2 pasos: instalar componente + restaurar mqtt.c.

---

## Archivos modificados

- `components/wifi/wifi.c` — Agregó inicialización de netif y event loop
- `components/sensors/sensor.c` — Aumentó timeout ultrasónico
- Binario nuevo: `build/nodo_cisterna.bin` ✓ (listo para flashear)
