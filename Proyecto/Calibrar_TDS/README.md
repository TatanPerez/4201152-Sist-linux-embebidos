# Calibrar_TDS

⚡ Proyecto para leer y calibrar valores TDS (Total Dissolved Solids) usando un microcontrolador ESP (ESP-IDF)

## 🔍 Descripción

Este proyecto usa una placa ESP (ESP32 u otra compatible con ESP-IDF) para leer el valor analógico de un sensor TDS (sonda de sólidos disueltos) mediante el ADC, aplicar una calibración de offset/ganancia y mostrar el valor en un formato de "ppm" *relativo*. Además integra una consola por UART para calibrar y persistir los parámetros en NVS (almacenamiento no volátil).

- Lectura de ADC (oneshot) con promedio
- Conversión a una unidad "ppm-like" con compensación de offset/gain
- Persistencia de la calibración mediante NVS
- Interfaz de consola via UART0 para calibración en tiempo real

---

## ✅ Características principales

- `tds` — Módulo principal de TDS: lectura raw/ppm, calibración y persistencia
- `adc_driver` — Abstracción para lectura ADC (oneshot API de ESP-IDF)
- `storage` — Persistencia de floats usando NVS
- Soporta comandos de calibración por consola: `calA`, `calB`, `save`, `show`

---

## 📁 Estructura del proyecto

- `main/main.c` — Ejemplo de uso, tareas FreeRTOS y consola
- `components/adc_driver/` — Driver ADC (configurable)
- `components/tds/` — Lógica TDS (lecturas, conversión, calibración)
- `components/storage/` — Manejo de NVS para guardar/calibration
- `sdkconfig` — Configuración de ESP-IDF
- `CMakeLists.txt`/`components/*/CMakeLists.txt` — Build con ESP-IDF

---

## 🔧 Requisitos de hardware

- Placa ESP32/ESP-IDF compatible (ESP32/ESP32-S2/...)
- Sonda de TDS con salida analógica conectada a una entrada ADC de la placa
- Cable USB para alimentación y UART (consola)

Nota: Ajusta `ADC_CHANNEL` y `DEFAULT_VREF` en `components/adc_driver/adc_driver.c` según tu placa.

---

## 🧰 Requisitos de software

- ESP-IDF (compatible con la versión usada en tu entorno). [Instrucciones oficiales de instalación](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) 

---

## 🚀 Cómo compilar y cargar (ESP-IDF)

1. Instalar y configurar ESP-IDF. Abre un terminal y ejecuta (si todavía no lo hiciste):

```bash
# Linux/macOS (ejemplo, según tu instalación de ESP-IDF)
. $HOME/esp/esp-idf/export.sh
```

2. Desde la raíz del proyecto:

```bash
cd /ruta/al/proyecto/Calibrar_TDS
idf.py set-target esp32   # si aplica a tu placa
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Reemplaza `/dev/ttyUSB0` por tu puerto USB serie en Linux.

---

## 🧾 Uso y comandos de consola

Abre el monitor serie (`idf.py monitor`) y usa la consola integrada (stdin) para interactuar. Los comandos disponibles (escribe el comando y presiona Enter):

- `calA` — Guardar la lectura actual como punto de calibración A (offset)
- `calB` — Guardar la lectura actual como punto de calibración B (para calcular ganancia)
- `save` — Guardar (persistir) la calibración en NVS
- `show` — Mostrar offset y gain actuales

### Ejemplo de calibración (flujo sugerido)

1. Llena la sonda con agua destilada / punto de referencia 0ppm (o la condición de referencia que uses).
2. Ejecuta `calA` — el sistema guardará el `offset` en RAM.
3. Llena la sonda con una solución de alto TDS conocida (p.ej. 1000 ppm) y ejecuta `calB`.
4. Ejecuta `save` para persistir la calibración.
5. Usa `show` para confirmar los valores guardados.

> Nota: la fórmula implementada en `components/tds/tds.c` calcula:
> normalized = (raw - offset) * gain
> tds_ppm = normalized * 1000.0f
>
> Esto define una escala arbitraria a "ppm-like"; ajusta la lógica o la constante de escala según tu sensor o método de calibración.

---

## 🧩 API (componentes principales)

A continuación un resumen de las funciones exportadas por los componentes:

### components/tds

- `void tds_init(void);` — Inicializa el módulo (carga calibración si existe)
- `float tds_read_raw(void);` — Lee el ADC (avg) y devuelve raw como float
- `float tds_read_ppm(void);` — Devuelve lectura convertida a ppm (relativo)
- `void tds_set_calibration_point_A(float raw);` — Establece offset (A)
- `void tds_set_calibration_point_B(float raw);` — Establece gain (B)
- `esp_err_t tds_save_calibration(void);` — Persiste offset/gain
- `esp_err_t tds_load_calibration(void);` — Carga offset/gain de NVS
- `float tds_get_offset(void);` — Devuelve offset actual
- `float tds_get_gain(void);` — Devuelve gain actual

### components/adc_driver

- `esp_err_t adc_init(void);` — Inicializa ADC (oneshot)
- `int adc_read_raw(int samples);` — Lee raw (0..4095) promedio
- `float adc_read_voltage(int samples);` — Convierte raw a milivoltios

### components/storage

- `esp_err_t storage_init(void);` — Inicializa NVS
- `esp_err_t storage_save_float(const char *key, float value);` — Guarda float
- `esp_err_t storage_load_float(const char *key, float *value);` — Carga float

---

## 🔧 Ajustes y personalización

- Cambiar el pin/entrada ADC: edita `components/adc_driver/adc_driver.c` y ajusta `ADC_CHANNEL` y `ADC_UNIT_ID` según tu placa.
- Cambiar el VREF para la conversión de raw a mV: modifica `DEFAULT_VREF`.
- Cambiar la escala de ppm: edita `tds_read_ppm()` en `components/tds/tds.c` y ajusta la constante de escala o añade mejor compensación de temperatura.
- Añadir más puntos de calibración o mejorar la fórmula: puedes extender `tds.c` con una curva polinómica o LUT.

---

## ⚠️ Problemas comunes y soluciones

- NVS no inicializa: el proyecto intenta borrar y reintentar si encuentra tablas llenas o versiones cambiadas. Si aún falla, revisa permisos o el soporte de NVS para tu placa.
- Lecturas ADC inconsistentes: asegúrate de tener un buen acondicionamiento de señal (filtrado, referencia estable, resistencia en serie) y ajustar `ADC_ATTEN` o `DEFAULT_VREF` si es necesario.
- UART/Consola: asegúrate de usar el puerto serie correcto; `idf.py monitor` se conecta por defecto a la consola UART.

---

## 🧪 Tests / Validación rápida

Puedes comprobar rápidamente que las lecturas funcionan:

1. Compila y monitoriza con `idf.py monitor`.
2. Observa los mensajes: `TDS raw=... ppm=...` apareciendo cada segundo.
3. Prueba los comandos `calA`, `calB`, `save`, `show` para verificar el comportamiento de calibración y persistencia.

---

## 🤝 Contribuciones

Si quieres colaborar, abre un issue o pull request en el repo. Algunas sugerencias de mejoras:

- Implementar compensación de temperatura real para TDS
- Agregar filtro digital de lecturas (IIR, median, etc.)
- Añadir soporte para múltiples canales o sondas
- Documentar y añadir tests unitarios y de integración

---

## 📄 Licencia

Añadir licencia (por ejemplo MIT) según la política del proyecto; actualmente no incluye archivo de licencia. Si deseas, puedo agregar un `LICENSE` con MIT (u otra).

---

## ℹ️ Contacto

Repo por: `Tatan-Perez` (owner)

Si deseas que adapte el README a algún estilo concreto (más técnico, más visual o con diagramas), dímelo y lo ajusto. 

---

¡Listo! Si quieres, también puedo:
- Agregar un `LICENSE` (MIT u otra).
- Añadir un `CONTRIBUTING.md` y un `Makefile` o `README` traducido a inglés.

