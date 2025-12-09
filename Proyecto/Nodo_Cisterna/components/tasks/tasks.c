#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "tasks.h"
#include "../sensors/sensor.h"

static const char *TAG = "TASKS";

// Declaración forward de función estática
static void task_sensor_read_loop(void *pvParameters);

// Variables globales
static shared_sensor_data_t g_sensor_data = {
    .sensor_data = {0},
    .mutex = NULL
};

static int g_pump_relay_pin = -1;
static bool g_pump_relay_state = false;

/**
 * @brief Inicializa el sistema de tareas FreeRTOS
 */
esp_err_t tasks_init(const task_config_t *config)
{
    if (config == NULL) {
        ESP_LOGE(TAG, "✗ Configuración de tareas es NULL");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "→ Inicializando sistema de tareas...");

    // ========== Crear semáforo mutex ==========
    g_sensor_data.mutex = xSemaphoreCreateMutex();
    if (g_sensor_data.mutex == NULL) {
        ESP_LOGE(TAG, "✗ Error creando mutex");
        return ESP_FAIL;
    }
    ESP_LOGD(TAG, "  ✓ Mutex creado");

    // ========== Inicializar sensores ==========
    esp_err_t ret = sensor_init(config->ultrasonic_trig_pin,
                                config->ultrasonic_echo_pin,
                                config->tds_adc_pin);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error inicializando sensores: %s", esp_err_to_name(ret));
        return ret;
    }

    // ========== Configurar pin del relé ==========
    g_pump_relay_pin = config->pump_relay_pin;
    
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << g_pump_relay_pin),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };

    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error configurando pin del relé: %s", esp_err_to_name(ret));
        return ret;
    }

    // Apagar bomba inicialmente
    gpio_set_level(g_pump_relay_pin, 0);
    g_pump_relay_state = false;
    ESP_LOGD(TAG, "  ✓ Pin del relé configurado (pin %d)", g_pump_relay_pin);

    // ========== Crear tarea de lectura de sensores ==========
    xTaskCreate(task_sensor_read_loop,
                "sensor_read_task",
                4096,                           // Stack size
                (void *)config,                  // Parámetros
                2,                              // Prioridad
                NULL);                          // Handle

    ESP_LOGI(TAG, "✓ Sistema de tareas inicializado");
    return ESP_OK;
}

/**
 * @brief Tarea FreeRTOS para lectura periódica de sensores
 * 
 * Esta tarea realiza lecturas cada 1 segundo y actualiza la estructura
 * compartida de forma sincronizada usando un semáforo mutex
 */
static void task_sensor_read_loop(void *pvParameters)
{
    const task_config_t *config = (const task_config_t *)pvParameters;
    
    ESP_LOGI(TAG, "→ Tarea de lectura de sensores iniciada");
    ESP_LOGI(TAG, "  Intervalo: %" PRIu32 " ms", config->sampling_interval_ms);

    sensor_data_t local_data;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        // Leer sensores
        esp_err_t err = sensor_read_all(&local_data);

        if (err == ESP_OK) {
            // Actualizar estructura compartida de forma segura
            if (xSemaphoreTake(g_sensor_data.mutex, pdMS_TO_TICKS(100))) {
                memcpy(&g_sensor_data.sensor_data, &local_data, 
                       sizeof(sensor_data_t));
                xSemaphoreGive(g_sensor_data.mutex);
            } else {
                ESP_LOGW(TAG, "⚠ Timeout adquiriendo mutex");
            }
        } else {
            ESP_LOGW(TAG, "⚠ Error leyendo sensores");
        }

        // Esperar al siguiente ciclo
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(config->sampling_interval_ms));
    }
}

/**
 * @brief Obtiene la estructura compartida de datos de sensores
 */
shared_sensor_data_t* tasks_get_shared_sensor_data(void)
{
    return &g_sensor_data;
}

/**
 * @brief Lee datos de sensores de forma segura (con semáforo)
 */
esp_err_t tasks_read_sensor_data(sensor_data_t *data, uint32_t timeout_ms)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (g_sensor_data.mutex == NULL) {
        ESP_LOGE(TAG, "✗ Mutex no inicializado");
        return ESP_ERR_INVALID_STATE;
    }

    // Intentar adquirir el mutex
    if (xSemaphoreTake(g_sensor_data.mutex, pdMS_TO_TICKS(timeout_ms))) {
        // Copiar datos
        memcpy(data, &g_sensor_data.sensor_data, sizeof(sensor_data_t));
        xSemaphoreGive(g_sensor_data.mutex);
        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "⚠ Timeout adquiriendo mutex (timeout=%" PRIu32 " ms)", timeout_ms);
        return ESP_ERR_TIMEOUT;
    }
}

/**
 * @brief Controla el relé de la bomba sumergible
 */
esp_err_t tasks_set_pump_relay(bool enable)
{
    if (g_pump_relay_pin < 0) {
        ESP_LOGE(TAG, "✗ Pin del relé no configurado");
        return ESP_ERR_INVALID_STATE;
    }

    // Evitar cambios innecesarios
    if (g_pump_relay_state != enable) {
        gpio_set_level(g_pump_relay_pin, enable ? 1 : 0);
        g_pump_relay_state = enable;
        
        ESP_LOGI(TAG, "→ Relé de bomba: %s", enable ? "ENCENDIDO" : "APAGADO");
    }

    return ESP_OK;
}

/**
 * @brief Obtiene el estado actual del relé de la bomba
 */
bool tasks_get_pump_relay_state(void)
{
    return g_pump_relay_state;
}
