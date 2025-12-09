#ifndef WIFI_H
#define WIFI_H

#include "esp_wifi.h"
#include "esp_event.h"

/**
 * @brief Inicializa y conecta el ESP32-C6 a la red Wi-Fi
 * 
 * @param ssid SSID de la red Wi-Fi
 * @param password Contraseña de la red Wi-Fi
 * @return esp_err_t ESP_OK si es exitoso, de lo contrario código de error
 */
esp_err_t wifi_init(const char *ssid, const char *password);

/**
 * @brief Obtiene el estado actual de la conexión Wi-Fi
 * 
 * @return bool true si está conectado, false en caso contrario
 */
bool wifi_is_connected(void);

/**
 * @brief Desconecta del Wi-Fi
 * 
 * @return esp_err_t ESP_OK si es exitoso
 */
esp_err_t wifi_disconnect(void);

#endif // WIFI_H
