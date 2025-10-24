// main.c
// App principal que inicializa sensores y ejecuta tests al arrancar.

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "sensor_ultrasonico.h"
#include "sensor_turbidez.h"

// Declaración de la función run_tests implementada en test/test_sensores.c
extern void run_tests(void);

static const char *TAG = "APP_MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando aplicación - inicializando sensores...");

    // Inicializa sensores
    sensor_ultrasonico_init();
    // sensor_turbidez_init();

    // Pequeña espera para estabilizar sensores
    vTaskDelay(pdMS_TO_TICKS(200));

    ESP_LOGI(TAG, "Ejecutando tests de sensores...");
    // Ejecuta la batería de tests (está compilada como parte del mismo binario)
    run_tests();

    // Después de tests, mantiene el dispositivo vivo; si se desea, puede reiniciarse.
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
