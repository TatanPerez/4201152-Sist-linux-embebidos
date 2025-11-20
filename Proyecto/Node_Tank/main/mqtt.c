// mqtt.c
// Implementación simple usando esp-mqtt

#include <string.h>
#include "esp_log.h"
#include "mqtt.h"
#include "esp_err.h"
#include "mqtt_client.h"

static const char *TAG = "MQTT_MODULE";
static esp_mqtt_client_handle_t client = NULL;

esp_err_t mqtt_init(const char *broker_uri)
{
    if (broker_uri == NULL) {
        ESP_LOGE(TAG, "broker_uri NULL");
        return ESP_ERR_INVALID_ARG;
    }

    esp_mqtt_client_config_t cfg = {
        .broker = {
            .address.uri = broker_uri,
        }
    };

    client = esp_mqtt_client_init(&cfg);
    if (client == NULL) {
        ESP_LOGE(TAG, "esp_mqtt_client_init returned NULL");
        return ESP_FAIL;
    }

    esp_err_t err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_mqtt_client_start failed: 0x%x", err);
        return err;
    }

    ESP_LOGI(TAG, "Cliente MQTT iniciado");
    return ESP_OK;
}

esp_err_t mqtt_publish(const char *topic, const char *payload)
{
    if (client == NULL) {
        ESP_LOGW(TAG, "Cliente MQTT no inicializado");
        return ESP_ERR_INVALID_STATE;
    }
    if (topic == NULL || payload == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    int msg_id = esp_mqtt_client_publish(client, topic, payload, 0, 1, 0);
    if (msg_id <= 0) {
        ESP_LOGW(TAG, "Fallo publicando en %s", topic);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Publicado id=%d topic=%s", msg_id, topic);
    return ESP_OK;
}
