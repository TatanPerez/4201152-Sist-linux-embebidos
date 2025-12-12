#ifndef MQTT_H
#define MQTT_H

#include "esp_err.h"
#include "mqtt_client.h"
#include <stdbool.h>

/**
 * @brief Estructura para configuración MQTT
 */
typedef struct {
    char broker_uri[256];       // URI del broker MQTT (ej: mqtt://192.168.1.100:1883)
    char client_id[32];          // ID del cliente MQTT
    char username[32];           // Nombre de usuario (opcional)
    char password[32];           // Contraseña (opcional)
} mqtt_config_t;

void* mqtt_init(const mqtt_config_t *config,
                esp_event_handler_t event_handler);

esp_err_t mqtt_connect(void *client);
esp_err_t mqtt_disconnect(void *client);

int mqtt_publish(void *client, const char *topic,
                 const char *data, int data_len, int qos, bool retain);

int mqtt_subscribe(void *client, const char *topic, int qos);

bool mqtt_is_connected(void *client);

void* mqtt_get_client(void);

#endif



