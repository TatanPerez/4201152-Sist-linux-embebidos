# Esquema de Conexiones - Nodo de Cisterna ESP32-C6

## Diagrama de Conexión

```
┌─────────────────────────────────────────────────────────────────┐
│                         ESP32-C6                                │
│                    Microcontrolador IoT                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  WiFi/BLE          │ GPIO/ADC              │ Power             │
│  (Integrado)       │                       │                   │
│                    │  GPIO-10 ─────────────┼──→ TRIG (HC-SR04) │
│                    │  GPIO-9  ─────────────┼──→ ECHO (HC-SR04) │
│                    │  ADC-0   ─────────────┼──→ Sensor TDS     │
│                    │  GPIO-8  ─────────────┼──→ Relé HW-307    │
│                    │                       │                   │
│                    │  GND ──────────────────┼──→ GND (sensores)│
│                    │  3.3V ─────────────────┼──→ VCC (sensores)│
│                    │  5V  ──────────────────┼──→ VCC (relé)    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Conexiones Detalladas

### 1. Sensor Ultrasónico HC-SR04

```
ESP32-C6           HC-SR04 (Sensor Ultrasónico)
────────────────   ───────────────────────────
GPIO-10 ───────→   VCC (5V) ⚠ Usar divisor de voltaje
GPIO-9  ←───────   ECHO
                   TRIG ←─── GPIO-10 
                   GND  ─── GND (ESP32-C6)
```

⚠️ **IMPORTANTE:** El sensor ultrasónico HC-SR04 opera con 5V, pero el ESP32-C6 
es de 3.3V. El pin ECHO debe conectarse a través de un divisor de voltaje:

```
Divisor de Voltaje para ECHO:
─────────────────────────────
HC-SR04 ECHO ──┬─── R1 (2.2kΩ) ──┬──→ GPIO-9 (ESP32-C6)
               │                 │
               └─── R2 (4.7kΩ) ──┴──→ GND
               
Voltaje en GPIO-9 = 5V × (4.7kΩ / (2.2kΩ + 4.7kΩ)) ≈ 3.3V ✓
```

### 2. Sensor TDS (Análogo)

```
ESP32-C6         Sensor TDS
────────────     ──────────
ADC-0 ─────────→ Salida analógica
GND ─────────→  GND
3.3V ────────→  VCC
```

**Rango de voltaje:** 0 - 3.3V (compatible directo)

### 3. Relé HW-307

```
ESP32-C6         Relé HW-307
────────────     ──────────────
GPIO-8 ────────→ IN (pin de control)
GND ────────→   GND
5V (USB) ──────→ VCC


Relé HW-307      Circuito de Bomba
─────────────    ─────────────────
COM ────────────→ Neutro (N) de bomba
NO (Normally ──→ Fase (L) de bomba
      Open)
```

⚠️ **CUIDADO:** El relé HW-307 maneja voltaje de CA (220V típico). 
Usar aislamiento eléctrico apropiado para seguridad.

---

## Diagrama de Flujo Eléctrico Completo

```
                    ┌──────────────────┐
                    │  Fuente de Poder │
                    │  USB 5V / 3.3V   │
                    └──┬──────────┬─────┘
                       │          │
                   (5V)│          │(3.3V)
                       │          │
        ┌──────────────┼────┐     │
        │   Divisor    │    │     │
        │  Voltaje R1  │    │     │
        │  R2=4.7kΩ    │    │     │
        │              │    │     │
    HC-SR04 ←─────────┘    │     │
    ├─ TRIG ─────────────  │  ┌──┴──────────┐
    ├─ ECHO ─────────────  │  │  ESP32-C6   │
    ├─ VCC ────────────────┤  ├─ GPIO-10   ─┤ TRIG
    └─ GND ────────────────┼─ ├─ GPIO-9    ←┤ ECHO
                           │  ├─ ADC-0     ←┤ TDS
        Sensor TDS ────────┤  ├─ GPIO-8    ─┤ Relé IN
        ├─ VCC ────────────┤  ├─ GND       ─┤ GND común
        ├─ GND ────────────┼──┤ 5V (USB)   ─┤ Relé VCC
        └─ OUT ────────────┘  └─────────────┘
                                    │
                                    │ WiFi 2.4GHz
                                    ↓
                            ┌──────────────────┐
                            │  Router WiFi     │
                            │  o Raspberry Pi  │
                            └──────────────────┘
                                    │
                                    │ MQTT (Puerto 1883)
                                    ↓
                            ┌──────────────────┐
                            │ Broker MQTT      │
                            │ (Mosquitto)      │
                            └──────────────────┘
```

---

## Requerimientos de Alimentación

### Consumo Típico por Componente

| Componente        | Voltaje | Corriente Típica | Potencia |
|-------------------|---------|-----------------|----------|
| ESP32-C6          | 3.3V    | 80 mA           | 264 mW   |
| Sensor Ultrasónico| 5V      | 15 mA           | 75 mW    |
| Sensor TDS        | 3.3V    | 5 mA            | 16.5 mW  |
| Relé HW-307       | 5V      | 100 mA (activo) | 500 mW   |
| **Total**         | -       | **~200 mA**     | **~855 mW** |

### Recomendación de Fuente

- **Mínimo:** 1A @ 5V (USB estándar)
- **Recomendado:** 2A @ 5V (mayor estabilidad)
- **Para bomba activa:** 10A @ 220V (CA) en el circuito secundario

---

## Lista de Componentes

### Componentes Electrónicos

| Componente                    | Cantidad | Especificación        |
|-------------------------------|----------|-----------------------|
| ESP32-C6                      | 1        | WiFi + Bluetooth      |
| Sensor Ultrasónico HC-SR04    | 1        | 40 kHz, 2-4m rango   |
| Sensor TDS                    | 1        | 0-1000 ppm, 3.3V     |
| Relé HW-307                   | 1        | 30A @ 220V CA        |
| Resistencia 2.2kΩ             | 1        | Divisor de voltaje   |
| Resistencia 4.7kΩ             | 1        | Divisor de voltaje   |
| Condensador 100nF             | 1        | Filtro de ruido      |
| Condensador 10µF              | 2        | Desacoplamiento      |

### Otros Materiales

| Material               | Cantidad | Descripción                    |
|----------------------|----------|--------------------------------|
| Cable USB             | 1        | Alimentación + Programación    |
| Cable de señal        | -        | AWG 22-26 para GPIO/ADC       |
| Cable de potencia     | -        | AWG 16-20 para bomba (220V)   |
| Tubería PVC           | -        | Conexiones agua (opcional)    |
| Protoboard            | 1        | Para prototipos               |
| Soldador + estaño     | -        | Conexiones (opcional)         |
| Tubo termorretráctil  | -        | Aislamiento de cables         |

---

## Instalación Física

### Ubicación Recomendada

```
┌─────────────────────────────────────────────┐
│         CISTERNA DE AGUA                    │
│                                             │
│    Sensor Ultrasónico (tapa)               │
│         ↓                                   │
│    ┌─────────────────────┐                 │
│    │   Nivel: 125 cm     │                 │
│    │   TDS: 450 ppm      │                 │
│    │                     │                 │
│    │ ░░░░░░░░░░░░░░░░░  │← Agua           │
│    │ ░░░░░░░░░░░░░░░░░  │                 │
│    │                     │                 │
│    │      Sensor TDS     │                 │
│    │         (sumergido) │                 │
│    └─────────────────────┘                 │
│              ↓ (tubo con válvula)          │
│    ┌──────────────────────┐                │
│    │  Bomba Sumergible    │ ← Controlada  │
│    │    (Relé HW-307)     │   por relé    │
│    └──────────────────────┘                │
└─────────────────────────────────────────────┘

ESP32-C6 + Sensores: En caja impermeable cerca de cisterna
Antena WiFi: Orientada hacia router (visible, sin obstáculos)
```

### Montaje de Sensores

**Sensor Ultrasónico HC-SR04:**
- Montar en la parte superior de cisterna
- Orientar hacia abajo perpendicular al agua
- Distancia mínima al agua: 2 cm
- Distancia máxima: 4 metros

**Sensor TDS:**
- Sumergir en el agua (al menos 5 cm de profundidad)
- Proteger del contacto con tuberías/bomba
- Limpiar regularmente para evitar depósitos

---

## Tabla de Pines GPIO Disponibles en ESP32-C6

```
Puerto JTAG   Arranque   Periféricos   Disponibles
─────────────────────────────────────────────────
GPIO-0  (I)   Arranque   -             ✓ (con limitaciones)
GPIO-1  ✓      -         UART0 TX      ✓
GPIO-2  (I)   -          -             ✓ (con limitaciones)
GPIO-3  ✓      -         UART0 RX      ✓
GPIO-4  ✓      -         SPI/ADC       ✓
GPIO-5  ✓      -         SPI/ADC       ✓
GPIO-6  ✓      -         SPI CS0       ✓
GPIO-7  ✓      -         SPI MOSI      ✓
GPIO-8  ✓      -         SPI MISO      ✓ (USADO: Relé)
GPIO-9  ✓      -         RXD1/I2C      ✓ (USADO: ECHO)
GPIO-10 ✓      -         TXD1/I2C      ✓ (USADO: TRIG)
GPIO-11 ✓      -         SPI CLK       ✓
GPIO-12 (I)   MTDI       JTAG          ✗ (Evitar)
GPIO-13 ✓      -         JTAG          ✓
GPIO-14 ✓      -         JTAG          ✓
GPIO-15 ✓      -         JTAG          ✓
GPIO-16 ✓      -         -             ✓
GPIO-17 ✓      -         UART1 TX      ✓
GPIO-18 ✓      -         UART1 RX      ✓
GPIO-19 ✓      -         USB D+        ✓
GPIO-20 ✓      -         USB D-        ✓

Pines ADC:
ADC1_CH0 = GPIO-0   ✓
ADC1_CH1 = GPIO-1   ✓
ADC1_CH2 = GPIO-2   ✓
ADC1_CH3 = GPIO-3   ✓
ADC1_CH4 = GPIO-4   ✓
ADC1_CH5 = GPIO-5   ✓ (USADO: TDS)
ADC1_CH6 = GPIO-6   ✓
ADC1_CH7 = GPIO-7   ✓

(I) = Tensión interna (no tocar)
✗   = No usar en proyecto actual
```

---

## Notas Importantes de Seguridad

⚠️ **CUIDADO ELÉCTRICO:**
- El relé HW-307 maneja voltaje de CA (típicamente 220V)
- Nunca tocar circuito de CA mientras está energizado
- Usar guantes aislantes para montaje
- Verificar polaridad antes de energizar

⚠️ **AGUA Y ELECTRICIDAD:**
- Mantener ESP32-C6 en caja impermeable/hermética
- Sensor TDS debe estar aislado de metal desnudo
- Cable de potencia alejado del agua

⚠️ **SOBRECALENTAMIENTO:**
- Relé HW-307 puede calentarse con cargas altas
- Dejar espacio para disipación de calor
- No cubrir completamente el relé

---

**Última actualización:** Diciembre 2025
**Versión:** 1.0
