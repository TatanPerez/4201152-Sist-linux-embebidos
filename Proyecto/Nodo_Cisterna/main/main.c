#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include <strings.h>

// Componentes locales
#include "wifi.h"
#include "mqtt.h"

#include "sensor.h"
#include "tasks.h"

#define WIFI_SSID       "RPi-Hotspot"        // Cambiar por el SSID de tu red Wi-Fi RPi-Hotspot
#define WIFI_PASSWORD   "12345678"    // Cambiar por la contraseña de tu red Wi-Fi
#define MQTT_BROKER_URI "mqtt://10.42.0.1:1883"  // Cambiar según broker 10.162.31.132  10.42.0.1     10.42.0.111
static const char *TAG = "CISTERNA_MAIN";

// Variables globales para configuración
static void *mqtt_client = NULL;
// Mode: central control via Node-RED by default
// Control modes
// Only MQTT-based control is used now; node-RED sends ON/OFF to control pump
static bool pump_manual_override = false;  // retained for compatibility (unused)

/**
 * @brief Callback para eventos MQTT
 * 
 * Maneja eventos de conexión, desconexión, suscripción y recepción de mensajes.
 * Particularmente, procesa comandos de control de bomba desde el topic "cistern_control"
 * 
 * Tópicos esperados:
 * - Recibir: cistern_control → mensajes "ON"/"OFF" para control de bomba
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, 
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    
    if (event_id == MQTT_EVENT_DATA) {
        // Mostrar topic y payload para diagnostico
        ESP_LOGI(TAG, "MQTT: topic=%.*s payload=%.*s", event->topic_len, event->topic,
                 event->data_len, event->data);
        // Procesar mensajes recibidos
        const char topic_control[] = "cistern_control";
        const char topic_pump_cmd[] = "cistern/pump_cmd"; // alias used by Node-RED flow
        bool is_control = false;

        if (event->topic_len == (int)sizeof(topic_control) - 1 &&
            memcmp(event->topic, topic_control, event->topic_len) == 0) {
            is_control = true;
        } else if (event->topic_len == (int)sizeof(topic_pump_cmd) - 1 &&
                   memcmp(event->topic, topic_pump_cmd, event->topic_len) == 0) {
            is_control = true;
        }

        if (is_control) {
            // Procesar comando de control de bomba (ON/OFF variants)
            char payload[32] = {0};
            int len = (event->data_len < (int)sizeof(payload) - 1) ? event->data_len : (int)sizeof(payload) - 1;
            strncpy(payload, event->data, len);
            payload[len] = '\0';

            ESP_LOGI(TAG, "-> Comando de bomba recibido: '%s'", payload);

            if (strcasecmp(payload, "ON") == 0 || strcasecmp(payload, "ENCENDER") == 0 || strcasecmp(payload, "1") == 0 || strcasecmp(payload, "TRUE") == 0) {
                esp_err_t rc = tasks_set_pump_relay(true);
                if (rc == ESP_OK) {
                    ESP_LOGI(TAG, "tasks_set_pump_relay(true) -> ESP_OK");
                } else {
                    ESP_LOGW(TAG, "tasks_set_pump_relay(true) -> %s", esp_err_to_name(rc));
                }
                ESP_LOGI(TAG, "OK Bomba encendida (desde Node-RED)");
            } else if (strcasecmp(payload, "OFF") == 0 || strcasecmp(payload, "APAGAR") == 0 || strcasecmp(payload, "0") == 0 || strcasecmp(payload, "FALSE") == 0) {
                esp_err_t rc = tasks_set_pump_relay(false);
                if (rc == ESP_OK) {
                    ESP_LOGI(TAG, "tasks_set_pump_relay(false) -> ESP_OK");
                } else {
                    ESP_LOGW(TAG, "tasks_set_pump_relay(false) -> %s", esp_err_to_name(rc));
                }
                ESP_LOGI(TAG, "OK Bomba apagada (desde Node-RED)");
            } else {
                ESP_LOGW(TAG, "Comando desconocido en cistern_control: '%s' (aceptados: ON/OFF)", payload);
            }

            // Enviar confirmación del estado de la bomba por MQTT
            if (mqtt_is_connected(mqtt_client)) {
                const char *pump_state_str = tasks_get_pump_relay_state() ? "ON" : "OFF";
                mqtt_publish(mqtt_client, "cistern/pump_state", pump_state_str, strlen(pump_state_str), 1, true);
            }
        }
    }
}

/**
 * @brief Tarea FreeRTOS para lectura de sensores y publicación de datos
 * 
 * Esta tarea:
 * 1. Lee los sensores (nivel de agua y TDS) cada 1 segundo
 * 2. Clasifica la calidad del agua
 * 3. Publica los datos en tópicos MQTT
 * 
 * Nota: El control de la bomba se realiza únicamente mediante comandos
 * MQTT recibidos en el tópico "cistern_control" (ON/OFF)
 */
static void sensor_read_and_publish_task(void *pvParameters)
{
    ESP_LOGI(TAG, "→ Iniciando tarea de lectura y publicación de sensores");
    
    const uint32_t publish_interval = 1000;  // 1 segundo
    sensor_data_t sensor_data;
    const size_t json_buf_sz = 512;
    char *json_payload = (char *) malloc(json_buf_sz);
    if (json_payload == NULL) {
        ESP_LOGE(TAG, "✗ No memory for JSON buffer");
        vTaskDelete(NULL);
        return;
    }
    
    while (1) {
        // Leer datos de sensores de forma segura (con semáforo)
        esp_err_t err = tasks_read_sensor_data(&sensor_data, pdMS_TO_TICKS(500));
        
        if (err == ESP_OK) {
            // Preparar datos de sensores
            const char *water_state_str[] = {"LIMPIA", "MEDIA", "SUCIA"};
            const char *pump_state_str = tasks_get_pump_relay_state() ? "ON" : "OFF";
            
            // Publicar en topicos separados si MQTT esta conectado
            if (mqtt_is_connected(mqtt_client)) {
                // 1. Publicar nivel de agua (en cm)
                snprintf(json_payload, json_buf_sz, "%.2f", sensor_data.water_level);
                mqtt_publish(mqtt_client, "cistern/water_level", json_payload, strlen(json_payload), 1, false);
                
                // 2. Publicar TDS (en ppm)
                snprintf(json_payload, json_buf_sz, "%.1f", sensor_data.tds_value);
                mqtt_publish(mqtt_client, "cistern/tds_value", json_payload, strlen(json_payload), 1, false);
                
                // 3. Publicar estado del agua (LIMPIA/MEDIA/SUCIA)
                snprintf(json_payload, json_buf_sz, "%s", water_state_str[sensor_data.water_state]);
                mqtt_publish(mqtt_client, "cistern/water_state", json_payload, strlen(json_payload), 1, false);
                
                // 4. Publicar estado de la bomba (ON/OFF)
                snprintf(json_payload, json_buf_sz, "%s", pump_state_str);
                mqtt_publish(mqtt_client, "cistern/pump_state", json_payload, strlen(json_payload), 1, true);

                    ESP_LOGD(TAG, "-> Datos publicados en topicos MQTT");

                    // Control automático interno removido: Node-RED controla la bomba mediante ON/OFF
            } else {
                ESP_LOGW(TAG, "X MQTT desconectado, datos no publicados");
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

// Console REPL task (defined as a proper C function instead of a C++ lambda)
static void console_repl_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "→ Console REPL task starting");
    /* Initialize console but avoid heavy linenoise features that consume significant stack.
       Use VFS UART driver and a small heap-allocated input buffer read via fgets(). */
    esp_console_config_t cfg = ESP_CONSOLE_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_console_init(&cfg) );
    esp_console_register_help_command();

    /* Enable VFS for the UART used by the console */
    esp_vfs_dev_uart_use_driver(CONFIG_CONSOLE_UART_NUM);
    /* Disable stdio buffering for immediate echo/read */
    setvbuf(stdin, NULL, _IONBF, 0);

    const size_t line_sz = 256;
    char *line = (char *) calloc(1, line_sz);
    if (line == NULL) {
        ESP_LOGE(TAG, "✗ No se pudo reservar memoria para la consola");
        vTaskDelete(NULL);
        return;
    }

    const char *prompt = "cisterna> ";
    while (true) {
        // Print prompt and read a line using fgets which allocates input on the heap
        fputs(prompt, stdout);
        fflush(stdout);
        if (fgets(line, line_sz, stdin) == NULL) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Strip newline
        size_t len = strlen(line);
        if (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[len-1] = '\0';
            if (len > 1 && line[len-2] == '\r') line[len-2] = '\0';
        }

        if (strlen(line) > 0) {
            int ret;
            esp_console_run(line, &ret);
        }
    }
    free(line);
}

// Minimal UART command parser task. Reads lines from UART0 and calls sensor handlers.
static void uart_command_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "→ UART command task starting");
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    // RX buffer 256, no TX buffer
    uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0);

    char line[128];
    int idx = 0;
    while (1) {
        uint8_t ch;
        int len = uart_read_bytes(UART_NUM_0, &ch, 1, pdMS_TO_TICKS(100));
        if (len > 0) {
            if (ch == '\r' || ch == '\n') {
                if (idx > 0) {
                    line[idx] = '\0';
                    ESP_LOGI(TAG, "UART CMD received: %s", line);
                    if (strcasecmp(line, "calA") == 0) {
                        sensor_do_calA();
                    } else if (strcasecmp(line, "calB") == 0) {
                        sensor_do_calB();
                    } else if (strcasecmp(line, "save") == 0) {
                        sensor_do_save();
                    } else if (strcasecmp(line, "show") == 0) {
                        sensor_do_show();
                    } else {
                        ESP_LOGI(TAG, "Unknown command: %s", line);
                    }
                    idx = 0;
                }
            } else {
                if (idx < (int)sizeof(line) - 1) {
                    line[idx++] = (char)ch;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
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
    uint32_t wifi_timeout = 30000; // 30 seconds timeout for Wi-Fi to obtain IP
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
        .broker_uri = MQTT_BROKER_URI,
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
        // Suscribirse al topico de control de bomba (sera procesado en mqtt_event_handler)
        mqtt_subscribe(mqtt_client, "cistern_control", 1);
        ESP_LOGI(TAG, "-> Suscrito a topico 'cistern_control' para recibir comandos desde Node-RED");
        // Publish initial retained state so Node-RED knows current state and mode
        const char *initial_pump_state = tasks_get_pump_relay_state() ? "ON" : "OFF";
        mqtt_publish(mqtt_client, "cistern/pump_state", initial_pump_state, strlen(initial_pump_state), 1, true);
        // Mode topic removed; only pump_state retained publish is provided
    }
    
    // 4. Inicializar sensores y tareas
    ESP_LOGI(TAG, "→ Inicializando sensores y tareas FreeRTOS...");
    task_config_t task_cfg = {
        .sampling_interval_ms = 1000,        // 1 segundo
        .ultrasonic_trig_pin = GPIO_NUM_10,  // Pin TRIG del sensor ultrasónico
        .ultrasonic_echo_pin = GPIO_NUM_11,   // Pin ECHO del sensor ultrasónico
        .tds_adc_pin = 0,                    // Canal ADC 0 del sensor TDS
        .pump_relay_pin = GPIO_NUM_1         // Pin del relé de la bomba
        // (botón deshabilitado) 
    };
    
    esp_err_t tasks_err = tasks_init(&task_cfg);
    if (tasks_err != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error al inicializar tareas: %s", esp_err_to_name(tasks_err));
        return;
    }
    
    // 5. Crear tarea de lectura y publicación
    // Increase stack for sensor task to reduce risk of stack overflow (allocations done on heap)
    xTaskCreate(sensor_read_and_publish_task, 
                "sensor_task", 
                8192,                      // Stack size (increased)
                NULL,                      // Parámetros
                3,                         // Prioridad (más alta)
                NULL);                     // Handle

    // Registrar callback para publicar el estado de la bomba cuando cambie
    extern void pump_state_change_cb(bool state);
    tasks_register_pump_state_cb(pump_state_change_cb);

    // Start minimal UART command task (reads lines and triggers sensor commands)
    xTaskCreate(uart_command_task, "uart_cmd", 3072, NULL, 2, NULL);

    // Start console REPL task (optional interactive console)
    xTaskCreate(console_repl_task, "console", 4096, NULL, 1, NULL);
    
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

/**
 * @brief Callback llamado desde el componente de tareas cuando cambia el estado del relé
 */
void pump_state_change_cb(bool state)
{
    ESP_LOGI(TAG, "Callback: pump_state changed -> %s", state ? "ON" : "OFF");
    if (mqtt_is_connected(mqtt_client)) {
        const char *pump_state_str = state ? "ON" : "OFF";
        mqtt_publish(mqtt_client, "cistern/pump_state", pump_state_str, strlen(pump_state_str), 1, true);
    }
}

/* Button event callback removed (button disabled) */
