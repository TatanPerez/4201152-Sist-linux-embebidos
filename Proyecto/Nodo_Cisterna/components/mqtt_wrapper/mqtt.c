#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mqtt.h"

static const char *TAG = "MQTT_STUB";

/**
    * @brief Variables globales para el cliente MQTT
    * NOTA: Requiere el componente MQTT de ESP-IDF
 */
static esp_mqtt_client_handle_t global_client = NULL;
static bool mqtt_connected = false;

static void internal_mqtt_event_handler(void *handler_args,
                                        esp_event_base_t base,
                                        int32_t event_id,
                                        void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch (event->event_id) {

        case MQTT_EVENT_CONNECTED:
            mqtt_connected = true;
            ESP_LOGI(TAG, "✓ Conectado al broker MQTT");
            break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_connected = false;
            ESP_LOGW(TAG, "✗ Desconectado del broker MQTT");
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "→ Mensaje recibido en %.*s: %.*s",
                     event->topic_len, event->topic,
                     event->data_len, event->data);
            break;

        default:
            ESP_LOGD(TAG, "Evento MQTT id=%d", event->event_id);
            break;
    }
}
/**
 * @brief Inicializa el cliente MQTT
 * NOTA: RImplementación stub - requiere instalar componente MQTT de ESP-IDF
 */
void* mqtt_init(const mqtt_config_t *config,
                esp_event_handler_t user_handler)
{
    if (!config) return NULL;

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = config->broker_uri,
        .credentials.client_id = config->client_id,
        .credentials.username = config->username,
        .credentials.authentication.password = config->password,
    };

    global_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!global_client) {
        ESP_LOGE(TAG, "✗ Error al inicializar MQTT");
        return NULL;
    }

    // Registrar handlers
    esp_mqtt_client_register_event(global_client,
                                   ESP_EVENT_ANY_ID,
                                   internal_mqtt_event_handler,
                                   NULL);

    if (user_handler) {
        esp_mqtt_client_register_event(global_client,
                                       ESP_EVENT_ANY_ID,
                                       user_handler,
                                       NULL);
    }

    return global_client;
}
/**
 * @brief Conecta al broker MQTT
 */
esp_err_t mqtt_connect(void *client)
{
    return esp_mqtt_client_start(client);
}
/**
 * @brief Desconecta del broker MQTT
 */
esp_err_t mqtt_disconnect(void *client)
{
    return esp_mqtt_client_stop(client);
}
/**
 * @brief Publica un mensaje en un topic MQTT
 */
int mqtt_publish(void *client, const char *topic,
                 const char *data, int data_len, int qos)
{
    return esp_mqtt_client_publish(client, topic, data, data_len, qos, false);
}
/**
 * @brief Se suscribe a un topic MQTT
 */
int mqtt_subscribe(void *client, const char *topic, int qos)
{
    return esp_mqtt_client_subscribe(client, topic, qos);
}
/**
 * @brief Verifica si el cliente está conectado
 */
bool mqtt_is_connected(void *client)
{
    return mqtt_connected;
}
/**
 * @brief Obtiene el handle global del cliente MQTT
 */
void* mqtt_get_client(void)
{
    return global_client;
}

