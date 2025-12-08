#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Componentes locales
#include "wifi.h"
#include "mqtt.h"

#include "sensor.h"
#include "tasks.h"

#define WIFI_SSID       "Casa de Tatan"        // Cambiar por el SSID de tu red Wi-Fi
#define WIFI_PASSWORD   "12312312380"    // Cambiar por la contraseña de tu red Wi-Fi
#define DEFAULT_MQTT_BROKER_URI "mqtt://10.162.31.132:1883"  // Cambiar según broker

static const char *TAG = "CISTERNA_MAIN";

// Variables globales para configuración
static void *mqtt_client = NULL;
static bool pump_manual_override = false;

/**
 * @brief Callback para eventos MQTT
 * 
 * Maneja eventos de conexión, desconexión, suscripción y recepción de mensajes.
 * Particularmente, procesa comandos de control de bomba desde el topic "cistern_control"
 * 
 * NOTA: Esta función está simplificada para usar con el stub MQTT
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, 
                               int32_t event_id, void *event_data)
{
    // Implementación simplificada para compatibilidad con stub MQTT
    ESP_LOGD(TAG, "Evento MQTT: event_id=%" PRId32, event_id);
}

/**
 * @brief Tarea FreeRTOS para lectura de sensores y publicación de datos
 * 
 * Esta tarea:
 * 1. Lee los sensores (nivel de agua y TDS) cada 1 segundo
 * 2. Clasifica la calidad del agua
 * 3. Implementa lógica automática de control de bomba
 * 4. Publica los datos en formato JSON al broker MQTT
 * 
 * Reglas de control automático de bomba:
 * - Si nivel bajo Y agua aceptable (≤600 ppm) → encender
 * - Si nivel alto O agua sucia (>600 ppm) → apagar
 */
static void sensor_read_and_publish_task(void *pvParameters)
{
    ESP_LOGI(TAG, "→ Iniciando tarea de lectura y publicación de sensores");
    
    const uint32_t publish_interval = 1000;  // 1 segundo
    sensor_data_t sensor_data;
    char json_payload[512];
    
    while (1) {
        // Leer datos de sensores de forma segura (con semáforo)
        esp_err_t err = tasks_read_sensor_data(&sensor_data, pdMS_TO_TICKS(500));
        
        if (err == ESP_OK) {
            // Lógica de control automático de bomba
            if (!pump_manual_override) {
                // Umbral de nivel bajo: 20cm, nivel alto: 180cm
                bool level_low = (sensor_data.water_level < 20.0f);
                bool level_high = (sensor_data.water_level > 180.0f);
                bool water_acceptable = (sensor_data.water_state != WATER_STATE_DIRTY);
                
                if (level_low && water_acceptable) {
                    // Encender bomba: nivel bajo y agua aceptable
                    tasks_set_pump_relay(true);
                } else if (level_high || !water_acceptable) {
                    // Apagar bomba: nivel alto o agua sucia
                    tasks_set_pump_relay(false);
                }
            }
            
            // Preparar datos en formato JSON
            const char *water_state_str[] = {"LIMPIA", "MEDIA", "SUCIA"};
            const char *pump_state_str = tasks_get_pump_relay_state() ? "ON" : "OFF";
            
            snprintf(json_payload, sizeof(json_payload),
                     "{"
                     "\"nivel_agua_cm\":%.2f,"
                     "\"tds_ppm\":%.1f,"
                     "\"estado_agua\":\"%s\","
                     "\"estado_bomba\":\"%s\","
                    "\"timestamp\":%" PRIu32 ""
                     "}",
                     sensor_data.water_level,
                     sensor_data.tds_value,
                     water_state_str[sensor_data.water_state],
                     pump_state_str,
                     sensor_data.timestamp);
            
            // Publicar datos si MQTT está conectado
            if (mqtt_is_connected(mqtt_client)) {
                int msg_id = mqtt_publish(mqtt_client, "cistern_sensordata", 
                                         json_payload, strlen(json_payload), 1);
                ESP_LOGD(TAG, "→ Publicado en cistern_sensordata (msg_id=%d)", msg_id);
            } else {
                ESP_LOGW(TAG, "✗ MQTT desconectado, datos no publicados");
            }
            
            // Log de información
            ESP_LOGI(TAG, "Lectura #%" PRIu32 " | Nivel: %.2f cm | TDS: %.1f ppm (%s) | Bomba: %s",
                     sensor_data.timestamp,
                     sensor_data.water_level,
                     sensor_data.tds_value,
                     water_state_str[sensor_data.water_state],
                     pump_state_str);
        } else {
            ESP_LOGE(TAG, "✗ Error al leer sensores: %s", esp_err_to_name(err));
        }
        
        vTaskDelay(pdMS_TO_TICKS(publish_interval));
    }
}

/**
 * @brief Inicialización de NVS Flash
 * 
 * El almacenamiento NVS es necesario para que funcionen correctamente
 * algunos componentes como Wi-Fi y MQTT
 */
static void nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Si NVS está lleno o es versión antigua, borrarlo
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

/**
 * @brief Función principal de la aplicación
 * 
 * Inicializa todos los subsistemas:
 * 1. NVS Flash
 * 2. Wi-Fi
 * 3. MQTT
 * 4. Sensores y tareas FreeRTOS
 */
void app_main(void)
{
    ESP_LOGI(TAG, "\n\n=== NODO DE CONTROL DE CISTERNA ===");
    ESP_LOGI(TAG, "ESP32-C6 | Sistema de monitoreo y control de cisterna");
    ESP_LOGI(TAG, "Sensores: Ultrasónico + TDS | Control: Relé HW-307");
    ESP_LOGI(TAG, "===================================\n");

    // ========== INICIALIZACIÓN DE COMPONENTES ==========
    
    // 1. Inicializar NVS
    ESP_LOGI(TAG, "→ Inicializando NVS Flash...");
    nvs_init();
    
    // 2. Inicializar Wi-Fi
    ESP_LOGI(TAG, "→ Inicializando Wi-Fi...");
    // Configurar con credenciales (cambiar según red local)
    esp_err_t wifi_err = wifi_init(WIFI_SSID, WIFI_PASSWORD);
    if (wifi_err != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error al inicializar Wi-Fi: %s", esp_err_to_name(wifi_err));
        // Continuar de todas formas para permitir debugging
    }
    
    // Esperar a que se conecte a Wi-Fi (máximo 10 segundos)
    uint32_t wifi_timeout = 10000;
    uint32_t start_time = esp_log_timestamp();
    while (!wifi_is_connected() && (esp_log_timestamp() - start_time) < wifi_timeout) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    if (wifi_is_connected()) {
        ESP_LOGI(TAG, "✓ Conectado a Wi-Fi");
    } else {
        ESP_LOGW(TAG, "✗ No se pudo conectar a Wi-Fi en 10 segundos");
    }
    
    // 3. Inicializar MQTT
    ESP_LOGI(TAG, "→ Inicializando MQTT...");
    mqtt_config_t mqtt_cfg = {
        .broker_uri = DEFAULT_MQTT_BROKER_URI,  // Cambiar según broker
        .client_id = "esp32c6_cisterna",
        .username = "",  // Opcional
        .password = ""   // Opcional
    };
    
    mqtt_client = mqtt_init(&mqtt_cfg, mqtt_event_handler);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "✗ Error al inicializar cliente MQTT");
        // Continuar para permitir operación offline
    } else {
        mqtt_connect(mqtt_client);
    }
    
    // 4. Inicializar sensores y tareas
    ESP_LOGI(TAG, "→ Inicializando sensores y tareas FreeRTOS...");
    task_config_t task_cfg = {
        .sampling_interval_ms = 1000,        // 1 segundo
        .ultrasonic_trig_pin = GPIO_NUM_5,  // Pin TRIG del sensor ultrasónico
        .ultrasonic_echo_pin = GPIO_NUM_18,   // Pin ECHO del sensor ultrasónico
        .tds_adc_pin = 0,                    // Canal ADC 0 del sensor TDS
        .pump_relay_pin = GPIO_NUM_8         // Pin del relé de la bomba
    };
    
    esp_err_t tasks_err = tasks_init(&task_cfg);
    if (tasks_err != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error al inicializar tareas: %s", esp_err_to_name(tasks_err));
        return;
    }
    
    // 5. Crear tarea de lectura y publicación
    xTaskCreate(sensor_read_and_publish_task, 
                "sensor_task", 
                4096,                      // Stack size
                NULL,                      // Parámetros
                3,                         // Prioridad (más alta)
                NULL);                     // Handle
    
    ESP_LOGI(TAG, "\n✓ INICIALIZACIÓN COMPLETADA");
    ESP_LOGI(TAG, "El sistema está en funcionamiento...\n");
    
    // ========== LOOP PRINCIPAL ==========
    // El sistema continúa funcionando a través de tareas FreeRTOS
    // Esta función puede monitorear memoria o ejecutar otras funciones
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        // Mostrar información cada 10 segundos
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10000));
        
        ESP_LOGI(TAG, "→ Estado: WiFi=%s | MQTT=%s | Relé=%s",
                 wifi_is_connected() ? "✓" : "✗",
                 mqtt_is_connected(mqtt_client) ? "✓" : "✗",
                 tasks_get_pump_relay_state() ? "ON" : "OFF");
        
        // Información de memoria
        uint32_t free_heap = esp_get_free_heap_size();
        uint32_t min_free_heap = esp_get_minimum_free_heap_size();
        ESP_LOGD(TAG, "  Memoria: Libre=%" PRIu32 " B | Mínima=%" PRIu32 " B", free_heap, min_free_heap);
    }
}
