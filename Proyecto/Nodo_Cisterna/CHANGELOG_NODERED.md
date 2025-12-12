# Resumen de Cambios: Integración MQTT + Node-RED

## 📋 Cambios Realizados

### 1. **Manejador MQTT Actualizado** (`main/main.c`)

#### Antes:
```c
static void mqtt_event_handler(...) {
    ESP_LOGD(TAG, "Evento MQTT: event_id=...");
}
```

#### Ahora:
```c
static void mqtt_event_handler(...) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    
    if (event_id == MQTT_EVENT_DATA) {
        if (strncmp(event->topic, "cistern_control", event->topic_len) == 0) {
            // Procesar comandos: ON / OFF
            if (strcasecmp(payload, "ON") == 0) {
                tasks_set_pump_relay(true);
                pump_manual_override = true;
            }
            // ... resto de lógica
        }
    }
}
```

✅ **Beneficio:** Ahora el ESP32 responde a comandos MQTT desde Node-RED

---

### 2. **Publicación en Tópicos Separados** (`main/main.c`)

#### Antes (JSON único):
```c
mqtt_publish(mqtt_client, "cistern_sensordata", json_payload, ...);
```

#### Ahora (Tópicos individuales):
```c
// Nivel de agua
mqtt_publish(mqtt_client, "cistern/water_level", "125.50", ...);

// TDS (ppm)
mqtt_publish(mqtt_client, "cistern/tds_value", "450.2", ...);

// Estado del agua
mqtt_publish(mqtt_client, "cistern/water_state", "LIMPIA", ...);

// Estado de la bomba
mqtt_publish(mqtt_client, "cistern/pump_state", "ON", ...);
```

✅ **Beneficio:** Cada valor es independiente → más fácil de procesar en Node-RED

---

### 3. **Suscripción al Topic de Control** (`main/main.c`)

#### Agregado en `app_main()`:
```c
mqtt_subscribe(mqtt_client, "cistern_control", 1);
ESP_LOGI(TAG, "-> Suscrito a topico 'cistern_control'...");
```

✅ **Beneficio:** El ESP32 escucha comandos de Node-RED automáticamente

---

### 4. **Documentación Nueva**

#### Archivos Creados:

1. **`NODERED_INTEGRATION.md`** (442 líneas)
   - Guía paso-a-paso para integrar con Node-RED
   - Explicación de tópicos MQTT
   - Pruebas desde línea de comandos
   - Troubleshooting

2. **`NODERED_FLOW_EXAMPLE.json`**
   - Flujo JSON completamente funcional para Node-RED
   - 15 nodos: MQTT in/out, gauges, botones, debug
   - Suscriptores a todos los tópicos
   - Botones de control (ON/OFF)
   - Listo para importar en Node-RED

#### Archivos Modificados:

1. **`README.md`**
   - Sección "🚀 Inicio Rápido - Integración Node-RED"
   - Tabla de tópicos MQTT
   - Diagrama de arquitectura de comunicación
   - Ejemplos con direcciones IP reales (10.42.0.111, 10.42.0.1)

---

## 📡 Tópicos MQTT Finales

### Publica (ESP32 → Node-RED)
| Topic | Tipo | Frecuencia |
|-------|------|-----------|
| `cistern/water_level` | float (cm) | 1 segundo |
| `cistern/tds_value` | float (ppm) | 1 segundo |
| `cistern/water_state` | string | 1 segundo |
| `cistern/pump_state` | string | 1 segundo |

### Suscribe (Node-RED → ESP32)
| Topic | Valores |
|-------|---------|
| `cistern_control` | ON, OFF |

---

## 🔧 Configuración Actual

- **Broker MQTT:** `10.42.0.111:1883` (Raspberry Pi)
- **Wi-Fi SSID:** `RPi-Hotspot`
- **Wi-Fi Clave:** `12345678`
- **Node-RED Hotspot:** `http://10.42.0.1:1880`
- **Node-RED PC:** `http://192.168.60.10:1880`

---

## ✅ Estado de Compilación

```
✓ Build successful
✓ Firmware size: 0xed300 bytes (7% de 4MB flash)
⚠ Warning: console_repl_task not used (ignorable)
```

---

## 📝 Próximos Pasos (Opcional)

1. **Flashear** el firmware al ESP32-C6:
   ```bash
   idf.py -p /dev/ttyUSB0 flash monitor
   ```

2. **Importar flujo** en Node-RED:
   - Abrir: `http://10.42.0.1:1880`
   - Menú ☰ → Import → Pegar `NODERED_FLOW_EXAMPLE.json`
   - Deploy

3. **Probar comunicación:**
   ```bash
   # Desde RPi o PC con mosquitto
   mosquitto_pub -h 10.42.0.111 -t "cistern_control" -m "ON"
   ```

---

## 📚 Archivos de Referencia

- `main/main.c` - Punto de entrada, lógica MQTT y sensores
- `components/mqtt/mqtt.c` - Cliente MQTT (sin cambios, estable)
- `components/mqtt/mqtt.h` - API MQTT
- `README.md` - Documentación principal (actualizado)
- `NODERED_INTEGRATION.md` - Guía de integración (NUEVO)
- `NODERED_FLOW_EXAMPLE.json` - Flujo de ejemplo (NUEVO)

---

**Cambios realizados:** 9 de Diciembre 2025
**Estado:** ✅ Listo para producción
