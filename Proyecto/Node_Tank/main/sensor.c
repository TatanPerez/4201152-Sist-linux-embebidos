// sensor.c
// Implementación que usa el sensor ultrasónico ya presente en el proyecto.

#include <stdio.h>
#include "esp_log.h"
#include "sensor.h"
#include "sensor_ultrasonico.h" // usa medir_distancia() si está disponible

static const char *TAG = "SENSOR_MODULE";

void sensor_init(void)
{
    // Si existe la implementación del ultrasónico, inicializarla.
    // sensor_ultrasonico_init está definida en sensor_ultrasonico.c
    sensor_ultrasonico_init();
    ESP_LOGI(TAG, "Sensor inicializado (ultrasonico)");
}

float sensor_read_level_cm(void)
{
    int d = medir_distancia();
    if (d < 0) {
        ESP_LOGW(TAG, "Error midiendo distancia (timeout)");
        return -1.0f;
    }
    // Aquí se asume que el sensor devuelve la distancia desde el sensor al agua
    // Si se requiere convertir a nivel de tanque (altura), aplicar la transformación adecuada.
    return (float)d;
}
