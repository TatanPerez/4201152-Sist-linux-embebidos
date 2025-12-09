# 🚀 INICIO RÁPIDO: ESP32 + Node-RED

## En 5 Minutos

### 1. ✅ Verificar Conexiones

```bash
# Verificar que ESP32 está conectado
idf.py monitor

# Buscar estas líneas:
# [WIFI] ✓ Conectado a Wi-Fi
# [MQTT_STUB] ✓ Conectado al broker MQTT
# [CISTERNA_MAIN] -> Suscrito a topico 'cistern_control'
```

### 2. 📱 Abrir Node-RED

**Opción A (desde Raspberry Pi/Hotspot):**
```
http://10.42.0.1:1880
```

**Opción B (desde PC):**
```
http://192.168.60.10:1880
```

### 3. 📥 Importar Flujo

1. Menú ☰ (arriba a la derecha)
2. **Import**
3. **Select a file to import**
4. Seleccionar: `NODERED_FLOW_EXAMPLE.json`
5. **Import**
6. **Deploy** (botón rojo arriba a la derecha)

### 4. 🎮 Probar Control

En Node-RED:
- Haz click en botón **ON** → La bomba se enciende
- Haz click en botón **OFF** → La bomba se apaga
- Haz click en botón **AUTO** → Vuelve a automático

Deberías ver en el monitor:
```
[CISTERNA_MAIN] -> Comando de bomba recibido: 'ON'
[CISTERNA_MAIN] OK Bomba encendida (desde Node-RED)
```

### 5. 📊 Ver Dashboard

En Node-RED:
- Haz click en ◆ (icono al lado de Deploy)
- Selecciona **Dashboard**
- Verás los medidores en tiempo real

---

## Prueba desde Terminal (Opcional)

```bash
# Abre otra terminal en la RPi

# Ver nivel de agua cada segundo
mosquitto_sub -h 10.42.0.111 -t "cistern/water_level"

# En otra terminal: Enviar comando
mosquitto_pub -h 10.42.0.111 -t "cistern_control" -m "ON"
```

---

## Tópicos MQTT

### Que ESP32 PUBLICA (cada 1 segundo):
```
cistern/water_level   →  125.50
cistern/tds_value     →  450.2
cistern/water_state   →  LIMPIA
cistern/pump_state    →  ON
```

### Que ESP32 ESCUCHA:
```
cistern_control  ←  ON / OFF / AUTO
```

---

## Si No Funciona

### Problema: No veo datos en Node-RED

**Solución:**
1. Verifica que ESP32 está encendido
2. Verifica que Raspberry Pi broker está corriendo:
   ```bash
   sudo systemctl status mosquitto
   ```
3. Prueba manualmente:
   ```bash
   mosquitto_sub -h 10.42.0.111 -t "cistern/water_level"
   ```

### Problema: Bomba no responde

**Solución:**
1. Verifica que Node-RED está conectado al broker
2. En Node-RED, abre el panel **Debug** (lado derecho)
3. Haz click en botón ON
4. Deberías ver un mensaje de depuración
5. Revisa logs del ESP32:
   ```bash
   idf.py -p /dev/ttyUSB0 monitor
   ```

### Problema: Node-RED no carga

**Solución:**
1. Reinicia Node-RED en la RPi:
   ```bash
   pm2 restart node-red
   # o
   sudo systemctl restart node-red
   ```
2. Espera 10 segundos
3. Vuelve a abrir la página en el navegador (F5)

---

## Archivos de Referencia

- `NODERED_INTEGRATION.md` - Guía detallada (leer si tienes problemas)
- `NODERED_FLOW_EXAMPLE.json` - Flujo importable
- `CHANGELOG_NODERED.md` - Cambios técnicos
- `README.md` - Documentación completa

---

## Comandos Útiles

```bash
# Ver todos los tópicos en tiempo real
mosquitto_sub -v -h 10.42.0.111 -t '#'

# Probar cada tópico
mosquitto_sub -h 10.42.0.111 -t "cistern/water_level"
mosquitto_sub -h 10.42.0.111 -t "cistern/tds_value"
mosquitto_sub -h 10.42.0.111 -t "cistern/water_state"
mosquitto_sub -h 10.42.0.111 -t "cistern/pump_state"

# Controlar desde terminal
mosquitto_pub -h 10.42.0.111 -t "cistern_control" -m "ON"
mosquitto_pub -h 10.42.0.111 -t "cistern_control" -m "OFF"
mosquitto_pub -h 10.42.0.111 -t "cistern_control" -m "AUTO"
```

---

**¡Listo! Tu sistema Cisterna + Node-RED debería funcionar ahora.**

Para dudas más específicas, consulta `NODERED_INTEGRATION.md`
