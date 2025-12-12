// main.c
// Nodo de Sensor de Tanque (ESP32-C6)
// Inicializa NVS, Wi‑Fi, MQTT y crea la tarea que lee el sensor y publica en MQTT cada 5s.

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "mqtt.h"
#include "sensor.h"
#include "tasks.h"

static const char *TAG = "TANK_NODE";

// Valores por defecto (modificar o usar menuconfig para parametrizar)
#define DEFAULT_WIFI_SSID "Casa de Tatan"//RPi-Hotspot" RPi-Hotspot
#define DEFAULT_WIFI_PASS "123123123"
#define DEFAULT_MQTT_BROKER_URI "mqtt://10.162.31.132:1883"  // 10.162.31.132        10.42.0.111

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Inicializando Wi-Fi...");
    // Conectar a la red (reemplazar SSID/PASS o modificar la función para leer de config)
    wifi_init_sta(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);

    ESP_LOGI(TAG, "Inicializando MQTT (%s)...", DEFAULT_MQTT_BROKER_URI);
    if (mqtt_init(DEFAULT_MQTT_BROKER_URI) != ESP_OK) {
        ESP_LOGE(TAG, "Fallo inicializando cliente MQTT");
    }

    // Inicializar sensor (internamente puede usar el HC-SR04 u otro)
    sensor_init();

    // Crear semáforo para sincronizar acceso a sensor/MQTT
    SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();
    if (xMutex == NULL) {
        ESP_LOGE(TAG, "No se pudo crear semáforo");
        return;
    }

    // Iniciar tareas: la tarea de sensor lee cada 5s y publica en "tank_sensordata"
    tasks_start(xMutex);

    // app_main devuelve; tareas FreeRTOS continúan ejecutándose
}
