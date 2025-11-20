// tasks.c
// Implementa la tarea que lee el sensor cada 5s y publica en MQTT "tank_sensordata"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "tasks.h"
#include "sensor.h"
#include "mqtt.h"

static const char *TAG = "TASKS_MODULE";
static const char *MQTT_TOPIC = "tank_sensordata";

static void sensor_task(void *arg)
{
    SemaphoreHandle_t mutex = (SemaphoreHandle_t)arg;
    char payload[128];

    while (1) {
        // Tomar semáforo antes de acceder al sensor y publicar
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
            float level = sensor_read_level_cm();
            if (level >= 0.0f) {
                // Crear payload JSON simple
                int len = snprintf(payload, sizeof(payload), "{\"level_cm\": %.2f}", level);
                if (len > 0 && len < (int)sizeof(payload)) {
                    esp_err_t res = mqtt_publish(MQTT_TOPIC, payload);
                    if (res != ESP_OK) {
                        ESP_LOGW(TAG, "mqtt_publish fallo");
                    }
                }
            } else {
                ESP_LOGW(TAG, "Lectura de sensor fallida");
            }
            xSemaphoreGive(mutex);
        } else {
            ESP_LOGW(TAG, "No se obtuvo semáforo para leer sensor");
        }

        // Esperar 5 segundos antes de la próxima lectura
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    vTaskDelete(NULL);
}

void tasks_start(SemaphoreHandle_t mutex)
{
    if (mutex == NULL) {
        ESP_LOGE(TAG, "tasks_start: mutex NULL");
        return;
    }

    BaseType_t ok = xTaskCreate(sensor_task, "sensor_task", 4096, mutex, 5, NULL);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "No se pudo crear sensor_task");
    } else {
        ESP_LOGI(TAG, "sensor_task creada");
    }
}
