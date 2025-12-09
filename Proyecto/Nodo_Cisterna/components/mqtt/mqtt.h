#ifndef MQTT_H
#define MQTT_H

#include "esp_err.h"

/**
 * @brief Estructura para configuración MQTT
 */
typedef struct {
    char broker_uri[256];        // URI del broker MQTT (ej: mqtt://192.168.1.100:1883)
    char client_id[32];          // ID del cliente MQTT
    char username[32];           // Nombre de usuario (opcional)
    char password[32];           // Contraseña (opcional)
} mqtt_config_t;

/**
 * @brief Inicializa el cliente MQTT
 * 
 * @param config Estructura con configuración MQTT
 * @param event_callback Callback para manejar eventos MQTT (void pointer)
 * @return void* Handle del cliente MQTT, NULL si falla
 */
void* mqtt_init(const mqtt_config_t *config, 
                void *event_callback);

/**
 * @brief Conecta al broker MQTT
 * 
 * @param client Handle del cliente MQTT
 * @return esp_err_t ESP_OK si es exitoso
 */
esp_err_t mqtt_connect(void *client);

/**
 * @brief Desconecta del broker MQTT
 * 
 * @param client Handle del cliente MQTT
 * @return esp_err_t ESP_OK si es exitoso
 */
esp_err_t mqtt_disconnect(void *client);

/**
 * @brief Publica un mensaje en un topic MQTT
 * 
 * @param client Handle del cliente MQTT
 * @param topic Topic MQTT de destino
 * @param data Datos a publicar
 * @param data_len Longitud de los datos
 * @param qos Nivel de QoS (0, 1 o 2)
 * @return int ID del mensaje publicado, -1 si falla
 */
int mqtt_publish(void *client, const char *topic, 
                 const char *data, int data_len, int qos);

/**
 * @brief Se suscribe a un topic MQTT
 * 
 * @param client Handle del cliente MQTT
 * @param topic Topic MQTT para suscribirse
 * @param qos Nivel de QoS (0, 1 o 2)
 * @return int ID de la suscripción, -1 si falla
 */
int mqtt_subscribe(void *client, const char *topic, int qos);

/**
 * @brief Verifica si el cliente está conectado al broker
 * 
 * @param client Handle del cliente MQTT
 * @return bool true si está conectado, false en caso contrario
 */
bool mqtt_is_connected(void *client);

/**
 * @brief Obtiene el handle del cliente MQTT (para acceso global)
 * 
 * @return void* Handle del cliente MQTT actual
 */
void* mqtt_get_client(void);

#endif // MQTT_H



