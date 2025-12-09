# 📑 ÍNDICE DE DOCUMENTACIÓN - CISTERNA + NODE-RED

## 🎯 EMPEZAR AQUÍ

### Si tienes 5 minutos ⏱️
→ **`QUICKSTART_NODERED.md`** (3.5 KB)
- Pasos rápidos para importar flujo
- Primeras pruebas
- Controlar bomba en 5 min

### Si tienes 15 minutos ⏱️⏱️
→ **`NODERED_INTEGRATION.md`** (6.2 KB)
- Guía completa de integración
- Explicación de tópicos MQTT
- Configuración paso-a-paso
- Troubleshooting

### Si necesitas referencia completa ⏱️⏱️⏱️
→ **`README.md`** (21 KB)
- Descripción general del proyecto
- Hardware y pines
- Estructura del código
- Todos los tópicos MQTT
- Instalación y compilación
- Pruebas y debugging

---

## 📚 DOCUMENTACIÓN DISPONIBLE

### Node-RED (NUEVO)
| Archivo | Tamaño | Propósito |
|---------|--------|----------|
| **QUICKSTART_NODERED.md** | 3.5 KB | Inicio rápido (5 min) |
| **NODERED_INTEGRATION.md** | 6.2 KB | Guía completa |
| **NODERED_FLOW_EXAMPLE.json** | 6.9 KB | Flujo importable |
| **CHANGELOG_NODERED.md** | 4.2 KB | Cambios técnicos |

### Proyecto General
| Archivo | Tamaño | Propósito |
|---------|--------|----------|
| **README.md** | 21 KB | Referencia principal |
| **QUICK_START.md** | 6.7 KB | Inicio rápido (general) |
| **INICIO.md** | 8.6 KB | Instrucciones iniciales |
| **CONFIGURACION.md** | 6.1 KB | Parámetros del sistema |
| **ESQUEMA_CONEXIONES.md** | 13 KB | Diagrama de pines |
| **DIAGNOSTICO_Y_CONFIGURACION.md** | 5.8 KB | Solución de problemas |
| **RESUMEN_PROYECTO.md** | 12 KB | Resumen ejecutivo |
| **CHECKLIST.md** | 8.0 KB | Lista de verificación |

---

## 🔍 BÚSQUEDA RÁPIDA POR TEMA

### 🤖 MQTT y Node-RED
- **¿Cómo conecto ESP32 con Node-RED?**
  → `NODERED_INTEGRATION.md` (Sección: "Configuración en Node-RED")

- **¿Qué tópicos MQTT hay?**
  → `README.md` (Sección: "Temas MQTT")
  → `NODERED_INTEGRATION.md` (Sección: "Tópicos MQTT")

- **¿Cómo importo el flujo de ejemplo?**
  → `QUICKSTART_NODERED.md` (Paso 3)

- **¿Cómo controlo la bomba desde Node-RED?**
  → `NODERED_INTEGRATION.md` (Sección: "Crear Nodos de Control")

### 💾 HARDWARE y PINES
- **¿Qué pines usados?**
  → `README.md` (Sección: "Pines GPIO Utilizados")
  → `ESQUEMA_CONEXIONES.md`

- **¿Cómo conectar sensores?**
  → `ESQUEMA_CONEXIONES.md`

### 🔧 COMPILACIÓN y FLASH
- **¿Cómo compilar el firmware?**
  → `README.md` (Sección: "Compilar el Firmware")
  → `QUICK_START.md`

- **¿Cómo cargar en ESP32?**
  → `README.md` (Sección: "Cargar Firmware")
  → `QUICK_START.md`

### 🐛 PROBLEMAS
- **No conecta a Wi-Fi**
  → `DIAGNOSTICO_Y_CONFIGURACION.md`
  → `NODERED_INTEGRATION.md` (Sección: "Troubleshooting")

- **MQTT no conecta**
  → `DIAGNOSTICO_Y_CONFIGURACION.md`
  → `README.md` (Sección: "Verificar Conexión MQTT")

- **Node-RED no carga**
  → `NODERED_INTEGRATION.md` (Sección: "Troubleshooting")

### 🎯 CONFIGURACIÓN
- **¿Cómo cambiar SSID y contraseña Wi-Fi?**
  → `CONFIGURACION.md`
  → `README.md` (Sección: "Credenciales Wi-Fi y MQTT")

- **¿Cómo cambiar IP del broker MQTT?**
  → `CONFIGURACION.md`
  → `main/main.c` (línea: `#define MQTT_BROKER_URI`)

- **¿Cómo cambiar pines GPIO?**
  → `README.md` (Sección: "Pines GPIO")
  → `main/main.c` (en `app_main()`)

---

## 📊 DIAGRAMA DE FLUJO RECOMENDADO

```
                 TU SITUACIÓN
                      ↓
         ┌────────────┴────────────┐
         ↓                         ↓
    ¿Tienes Node-RED?         ¿Solo ESP32?
         ↓                         ↓
    START_AQUÍ:              START_AQUÍ:
    QUICKSTART_              README.md
    NODERED.md               QUICK_START.md
         ↓                         ↓
    5 minutos                30 minutos
    + NODERED_              + entender
     INTEGRATION.md          arquitectura
         ↓                         ↓
    DASHBOARD!              SISTEMA
                           FUNCIONANDO
```

---

## 🚀 FLUJO DE TRABAJO RECOMENDADO

### Primera vez (Completo):
1. Lee `README.md` (Descripción General + Requisitos)
2. Lee `ESQUEMA_CONEXIONES.md` (cómo conectar hardware)
3. Lee `QUICK_START.md` (instalación)
4. Compila y flashea
5. Lee `NODERED_INTEGRATION.md` (integración)
6. Importa flujo y prueba

### Cambios en el futuro:
1. Modifica `main/main.c` o componentes
2. Compila: `idf.py build`
3. Flashea: `idf.py flash`
4. Prueba en monitor: `idf.py monitor`

### Debugging:
1. Abre monitor: `idf.py monitor`
2. Busca mensajes clave (ver `README.md`)
3. Consulta `DIAGNOSTICO_Y_CONFIGURACION.md`

---

## 💡 ARCHIVOS MÁS IMPORTANTES

### Código:
```
main/main.c                    ← APP PRINCIPAL (modificado)
components/mqtt/mqtt.c         ← CLIENTE MQTT (estable)
components/mqtt/mqtt.h         ← INTERFAZ MQTT
components/sensors/sensor.c    ← LECTURAS DE SENSORES
components/wifi/wifi.c         ← CONEXIÓN Wi-Fi
```

### Documentación:
```
README.md                      ← REFERENCIA PRINCIPAL
NODERED_INTEGRATION.md         ← SI USAS NODE-RED
QUICKSTART_NODERED.md          ← SI TIENES PRISA
ESQUEMA_CONEXIONES.md          ← SI NECESITAS PINES
```

---

## 📈 CAMBIOS RECIENTES (Diciembre 9, 2025)

### ✅ Completado:
- Manejador MQTT que procesa comandos
- Publicación en 4 tópicos separados
- Suscripción a `cistern_control`
- Documentación Node-RED completa
- Flujo JSON importable
- Guías paso-a-paso

### 🔄 Estable:
- Cliente MQTT
- Conexión Wi-Fi
- Sensores (ultrasónico + TDS)
- Calibración TDS

### 📊 Estado:
- Build: ✅ SUCCESS
- Firmware: ✅ 0xed300 bytes (7% libre)
- Tests: ✅ MQTT funciona
- Documentación: ✅ COMPLETA

---

## 🎓 NIVELES DE COMPLEJIDAD

### 🟢 BÁSICO (Leer primero):
- `QUICKSTART_NODERED.md` - En 5 minutos
- `README.md` - Descripción general
- Compila y flashea

### 🟡 INTERMEDIO (Después):
- `NODERED_INTEGRATION.md` - Entender integración
- `ESQUEMA_CONEXIONES.md` - Cómo conectar hardware
- Pruebas manuales con MQTT

### 🔴 AVANZADO (Personalización):
- Modifica `main/main.c` para cambios
- `CONFIGURACION.md` - Parámetros
- `DIAGNOSTICO_Y_CONFIGURACION.md` - Debugging

---

## 📞 AYUDA RÁPIDA

| Pregunta | Respuesta |
|----------|-----------|
| ¿Puedo empezar ya? | Sí, lee `QUICKSTART_NODERED.md` |
| ¿Es complicado? | No, son 5 pasos simples |
| ¿Necesito Node-RED? | No es obligatorio, pero recomendado |
| ¿Funciona sin Wi-Fi? | Sí, pero sin MQTT (offline) |
| ¿Puedo cambiar pines? | Sí, en `main/main.c` y `CONFIGURACION.md` |
| ¿Cuánto tarda compilar? | ~2 minutos |
| ¿Flashear es fácil? | Sí: `idf.py flash` |

---

## 📝 PLANTILLA DE BÚSQUEDA

Si no encuentras algo, busca palabras clave:

```bash
# En terminal, desde raíz del proyecto:
grep -r "palabra_clave" *.md
grep -r "SSID\|PASSWORD" main/main.c
grep -r "mqtt_publish" components/mqtt/
```

---

**Última actualización:** 9 de Diciembre 2025

**Autor:** Sistema de Control de Cisterna IoT

**Estado:** ✅ LISTO PARA PRODUCCIÓN
