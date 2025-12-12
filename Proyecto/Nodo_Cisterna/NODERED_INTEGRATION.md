# Guía de Integración: ESP32-C6 + Node-RED en Raspberry Pi

## Resumen Rápido

El ESP32-C6 (Nodo Cisterna) publica datos de sensores en **tópicos MQTT separados** y se suscribe a comandos de control desde **Node-RED** en la Raspberry Pi.

### Broker MQTT
- **Dirección:** `10.42.0.111:1883` (Raspberry Pi)
- **Red:** `RPi-Hotspot` (SSID)
- **Clave:** `12345678`

---

## Tópicos MQTT

### 📤 Publicación (Datos de Sensores)

El ESP32 publica **cada 1 segundo** en los siguientes tópicos:

| Tópico | Tipo | Ejemplo | Descripción |
|--------|------|---------|-------------|
| `cistern/water_level` | float | `125.50` | Nivel de agua en centímetros |
| `cistern/tds_value` | float | `450.2` | Conductividad en ppm |
| `cistern/water_state` | string | `LIMPIA` | LIMPIA \| MEDIA \| SUCIA |
| `cistern/pump_state` | string | `ON` | ON \| OFF |

### 📥 Suscripción (Control de Bomba)

Node-RED envía comandos a:

| Tópico | Valores | Efecto |
|--------|---------|--------|
| `cistern_control` | `ON` | Enciende bomba |
| `cistern_control` | `OFF` | Apaga bomba |

---

## Configuración en Node-RED

### 1. Acceso a Node-RED

- **Desde Raspberry Pi (hotspot):** http://10.42.0.1:1880
- **Desde PC (si está en red):** http://192.168.60.10:1880

### 2. Crear Nodos de Suscripción

#### A. Nivel de Agua

```
Node Type: MQTT in
Topic: cistern/water_level
QoS: 1
Name: "Nivel de Agua"
```

**Conectar a:**
- `gauge` (widget) para mostrar nivel (0-200 cm)
- `debug` (para verificar lecturas)

#### B. TDS (Conductividad)

```
Node Type: MQTT in
Topic: cistern/tds_value
QoS: 1
Name: "TDS"
```

**Conectar a:**
- `gauge` (widget) para mostrar ppm (0-1000)
- `debug`

#### C. Estado del Agua

```
Node Type: MQTT in
Topic: cistern/water_state
QoS: 1
Name: "Estado del Agua"
```

**Conectar a:**
- `text` (widget) para mostrar estado
- `debug`

#### D. Estado de la Bomba

```
Node Type: MQTT in
Topic: cistern/pump_state
QoS: 1
Name: "Estado Bomba"
```

**Conectar a:**
- `text` (widget)
- `debug`

### 3. Crear Nodos de Control de Bomba

#### Botón: Encender Bomba (UI button — Node-RED Dashboard)

```
Node Type: ui_button
Label: "ON"
Payload: ON
Topic: (dejar vacío)
Output to: mqtt_out (siguiente paso)
```

#### Botón: Apagar Bomba (UI button — Node-RED Dashboard)

```
Node Type: ui_button
Label: "OFF"
Payload: OFF
```

#### Note: Modo automático

El firmware ya no implementa un modo `AUTO` interno ni un botón de hardware. Para control automático o manual, crea una lógica en Node-RED que publique `ON`/`OFF` en `cistern_control` basándose en reglas (nivel y/o calidad de agua). En la interfaz del dashboard, puedes usar `ui_button` para publicar `ON`/`OFF`.

### 4. Nodo MQTT Out para Enviar Comandos

```
Node Type: MQTT out
Topic: cistern_control
QoS: 1
Broker: 10.42.0.111:1883
```

**Conectar los botones anteriores → este nodo**

---

## Pasos de Instalación Rápida

### 1. Instalar Dependencias en Node-RED (si es necesario)

En la terminal de la Raspberry Pi:

```bash
cd ~/.node-red
npm install node-red-dashboard
npm install node-red-contrib-mqtt-broker
```

Reiniciar Node-RED:
```bash
pm2 restart node-red
# o
node-red-stop && node-red-start
```

### 2. Importar Flujo de Ejemplo

1. Abrir Node-RED: http://10.42.0.1:1880
2. Menú ☰ → Import → Paste JSON
3. Pegar contenido de `NODERED_FLOW_EXAMPLE.json`
4. Click Deploy

### 3. Configurar Broker MQTT

En Node-RED:
1. Doble-click en cualquier nodo MQTT
2. Servidor: `10.42.0.111`
3. Puerto: `1883`
4. Update

### 4. Desplegar y Probar

```
Deploy → Abrir Dashboard (http://10.42.0.1:1880/ui)
```

---

## Pruebas desde Línea de Comandos

### Test 1: Suscribirse a Datos de Sensores

```bash
# Terminal 1: Ver nivel de agua
mosquitto_sub -h 10.42.0.111 -t "cistern/water_level"

# Terminal 2: Ver TDS
mosquitto_sub -h 10.42.0.111 -t "cistern/tds_value"

# Terminal 3: Ver estado agua
mosquitto_sub -h 10.42.0.111 -t "cistern/water_state"

# Terminal 4: Ver estado bomba
mosquitto_sub -h 10.42.0.111 -t "cistern/pump_state"
```

**Esperado:** Recibirás valores cada 1 segundo

### Test 2: Enviar Comandos de Control

```bash
# Encender bomba
mosquitto_pub -h 10.42.0.111 -t "cistern_control" -m "ON"

# Apagar bomba
mosquitto_pub -h 10.42.0.111 -t "cistern_control" -m "OFF"

# Comandos de control
mosquitto_pub -h 10.42.0.111 -t "cistern_control" -m "ON"
mosquitto_pub -h 10.42.0.111 -t "cistern_control" -m "OFF"
```

**Esperado:** 
- Log en ESP32: `OK Bomba encendida (desde Node-RED)`
- Estado de relay cambia inmediatamente

---

## Lógica de Control Automático (impleméntalo en Node-RED)

En lugar de enviar un comando `AUTO` al ESP, implementa la lógica en Node-RED que publique `ON` o `OFF` según las reglas de tu cisterna.

Ejemplo de regla en Node-RED:

```
SI nivel_agua < 20 cm Y tds_ppm ≤ 600 → publicar `ON`
SI nivel_agua > 180 cm O tds_ppm > 600 → publicar `OFF`
```

**Mientras está en `ON` o `OFF` (manual):**
- La bomba obedece el comando sin considerar sensores

---

## Archivos de Referencia

- `main/main.c` - Lógica MQTT y control de bomba
- `components/mqtt/mqtt.c` - Implementación cliente MQTT
- `README.md` - Documentación completa
- `NODERED_FLOW_EXAMPLE.json` - Flujo de ejemplo (importar en Node-RED)

---

## Troubleshooting

### Problema: Node-RED no recibe datos

1. Verificar conexión Wi-Fi del ESP32: `idf.py monitor`
2. Verificar broker MQTT en RPi: `mosquitto -v`
3. Probar publicación manual:
   ```bash
   mosquitto_pub -h 10.42.0.111 -t "cistern/water_level" -m "100.5"
   ```

### Problema: Bomba no responde a comandos

1. Verificar que Node-RED está publicando en `cistern_control`
2. Revisar logs: `idf.py monitor` buscar "Comando de bomba recibido"
3. Probar comando manual:
   ```bash
   mosquitto_pub -h 10.42.0.111 -t "cistern_control" -m "ON"
   ```

### Problema: Dashboard no actualiza

1. Actualizar página (F5)
2. Verificar pestañas en Node-RED (panel derecho)
3. Revisar nodos `debug` para ver si llegan mensajes

---

## Cambios Recientes en el Código

### `main/main.c`
- ✅ `mqtt_event_handler()` ahora procesa mensajes de `cistern_control`
- ✅ Publica en tópicos separados: `cistern/water_level`, `cistern/tds_value`, etc.
- ✅ Se suscribe automáticamente a `cistern_control` al conectar

### `components/mqtt/mqtt.c`
- ✅ Mantiene la implementación estable del cliente MQTT

---

## Contacto / Soporte

Si tienes problemas con la integración:
1. Revisar logs del ESP32: `idf.py -p /dev/ttyUSB0 monitor`
2. Revisar logs de Node-RED en RPi: `pm2 logs node-red`
3. Verificar tráfico MQTT: `mosquitto_sub -v -h 10.42.0.111 -t '#'`

---

**Última actualización:** Diciembre 9, 2025
