#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi.h"

static const char *TAG = "WIFI";

// Event group para el estado de conexión Wi-Fi
static EventGroupHandle_t wifi_event_group = NULL;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int retry_count = 0;
static const int MAXIMUM_RETRY = 5;

/**
 * @brief Event handler para eventos de Wi-Fi
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            ESP_LOGI(TAG, "→ Iniciando conexión a Wi-Fi...");
            esp_wifi_connect();
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            if (retry_count < MAXIMUM_RETRY) {
                ESP_LOGI(TAG, "→ Intentando reconectar a Wi-Fi... (intento %d/%d)",
                         retry_count + 1, MAXIMUM_RETRY);
                esp_wifi_connect();
                retry_count++;
            } else {
                xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
                ESP_LOGE(TAG, "✗ No se pudo conectar a Wi-Fi después de %d intentos",
                         MAXIMUM_RETRY);
            }
        }
    } else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG, "✓ Conectado a Wi-Fi | IP obtenida: " IPSTR,
                     IP2STR(&event->ip_info.ip));
            retry_count = 0;
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        }
    }
}

/**
 * @brief Inicializa Wi-Fi en modo estación
 */
esp_err_t wifi_init(const char *ssid, const char *password)
{
    if (ssid == NULL || password == NULL) {
        ESP_LOGE(TAG, "✗ SSID y contraseña no pueden ser NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Crear event group
    wifi_event_group = xEventGroupCreate();
    if (wifi_event_group == NULL) {
        ESP_LOGE(TAG, "✗ Error al crear event group");
        return ESP_FAIL;
    }

    // Inicializar netif
    // Inicializar TCP/IP stack and event loop if not already done
    esp_err_t ret = esp_netif_init();
    if (ret != ESP_OK && ret != ESP_ERR_NO_MEM && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "✗ Error inicializando esp_netif: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "✗ Error creando event loop por defecto: %s", esp_err_to_name(ret));
        return ret;
    }

    if (esp_netif_get_handle_from_ifkey("WIFI_STA_DEF") == NULL) {
        esp_netif_create_default_wifi_sta();
    }

    // Crear y configurar Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error al inicializar Wi-Fi: %s", esp_err_to_name(ret));
        return ret;
    }

    // Registrar event handlers
    ret = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                     &wifi_event_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error al registrar handler de WIFI_EVENT");
        return ret;
    }

    ret = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                     &wifi_event_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error al registrar handler de IP_EVENT");
        return ret;
    }

    // Configurar credenciales
    wifi_config_t wifi_config = {};
    
    // Configurar parámetros de seguridad
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

    // Copiar SSID y contraseña
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0';
    
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0';

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error al configurar modo STA");
        return ret;
    }

    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error al configurar credenciales Wi-Fi");
        return ret;
    }

    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error al iniciar Wi-Fi");
        return ret;
    }

    ESP_LOGI(TAG, "✓ Wi-Fi inicializado correctamente");
    return ESP_OK;
}


/**
 * @brief Obtiene estado actual de conexión Wi-Fi
 */
bool wifi_is_connected(void)
{
    if (wifi_event_group == NULL) {
        return false;
    }

    EventBits_t bits = xEventGroupGetBits(wifi_event_group);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}

/**
 * @brief Desconecta del Wi-Fi
 */
esp_err_t wifi_disconnect(void)
{
    if (wifi_is_connected()) {
        return esp_wifi_disconnect();
    }
    return ESP_OK;
}
