// mqtt.h
// API mínimo para inicializar y usar el cliente MQTT (esp-mqtt)

#ifndef MQTT_H
#define MQTT_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Inicializa y arranca el cliente MQTT. `broker_uri` ejemplo: "mqtt://192.168.1.10:1883"
esp_err_t mqtt_init(const char *broker_uri);

// Publica un mensaje (payload NUL-terminated) en `topic`. QoS=1 por defecto.
// Retorna ESP_OK si la publicación fue enviada al cliente MQTT.
esp_err_t mqtt_publish(const char *topic, const char *payload);

#ifdef __cplusplus
}
#endif

#endif // MQTT_H
