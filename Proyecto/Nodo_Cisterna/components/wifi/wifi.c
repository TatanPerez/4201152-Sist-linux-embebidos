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

static const char *wifi_reason_to_str(int reason);
static void delayed_connect_task(void *arg);

// Event group para el estado de conexión Wi-Fi
static EventGroupHandle_t wifi_event_group = NULL;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int retry_count = 0;
static const int MAXIMUM_RETRY = 10;
static char saved_ssid[33] = {0};

/**
 * @brief Event handler para eventos de Wi-Fi
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            ESP_LOGI(TAG, "→ Iniciando conexión a Wi-Fi...");
            // Ejecutar escaneo de diagnóstico ANTES de intentar conectar (si el SSID objetivo fue guardado)
            if (saved_ssid[0] != '\0') {
                wifi_scan_and_report(saved_ssid);
            } else {
                wifi_scan_and_report(NULL);
            }
            // Para evitar problemas si el AP aún no está listo, lanzamos la conexión
            // con un pequeño retraso en una tarea separada para no bloquear el event loop.
            xTaskCreate(&delayed_connect_task, "delayed_conn", 2048, NULL, 5, NULL);
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            // Registrar razón de desconexión si está disponible (ayuda en diagnostics)
            wifi_event_sta_disconnected_t *disc = (wifi_event_sta_disconnected_t *) event_data;
            if (disc) {
                ESP_LOGW(TAG, "X Wi-Fi desconectado (razón=%d). %s", disc->reason, wifi_reason_to_str(disc->reason));
            } else {
                ESP_LOGW(TAG, "X Wi-Fi desconectado (razón desconocida).");
            }

            // Si MAXIMUM_RETRY <= 0 → reintento infinito, si >0 respeta el límite
            if (MAXIMUM_RETRY > 0 && retry_count >= MAXIMUM_RETRY) {
                xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
                ESP_LOGE(TAG, "✗ No se pudo conectar a Wi-Fi después de %d intentos", MAXIMUM_RETRY);
            } else {
                esp_wifi_connect();
                retry_count++;
                ESP_LOGI(TAG, "→ Intentando reconectar a Wi-Fi... (intento %d)", retry_count);
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
    // No forzamos un modo de autenticación específico para evitar problemas
    // con APs que usen configuraciones WPA3-only o modos mixtos.

    // Copiar SSID y contraseña
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0';
    
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0';

    // Mostrar SSID configurado para debugging (no imprimir contraseña en claro)
    ESP_LOGI(TAG, "→ SSID configurado: '%s'", wifi_config.sta.ssid);

    // Guardar el SSID configurado para el handler de eventos (debug/scan)
    strncpy(saved_ssid, (const char *)wifi_config.sta.ssid, sizeof(saved_ssid) - 1);
    saved_ssid[sizeof(saved_ssid) - 1] = '\0';

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error al configurar modo STA");
        return ret;
    }

    // Desactive el modo de ahorro de energía STA (evita problemas de handshake en algunos APs)
    esp_err_t ps_ret = esp_wifi_set_ps(WIFI_PS_NONE);
    if (ps_ret == ESP_OK) {
        ESP_LOGI(TAG, "→ Power save modo: DISABLED");
    } else {
        ESP_LOGW(TAG, "→ No se pudo desactivar power save (ret=%s)", esp_err_to_name(ps_ret));
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

    // Hacer un escaneo rápido y reportar si el SSID objetivo está visible (útil para debugging)
    wifi_scan_and_report((const char *)wifi_config.sta.ssid);

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

void wifi_scan_and_report(const char *expected_ssid)
{
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };

    ESP_LOGI(TAG, "→ Ejecutando escaneo Wi-Fi para diagnostico...");
    esp_err_t err = esp_wifi_scan_start(&scan_config, true); // block until done
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "✗ Error iniciando escaneo: %s", esp_err_to_name(err));
        return;
    }

    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    if (ap_count == 0) {
        ESP_LOGW(TAG, "⌁ Escaneo completado: No se encontraron APs");
        return;
    }

    wifi_ap_record_t *ap_list = calloc(ap_count, sizeof(wifi_ap_record_t));
    if (ap_list == NULL) {
        ESP_LOGW(TAG, "✗ No hay memoria para la lista de APs");
        return;
    }

    esp_wifi_scan_get_ap_records(&ap_count, ap_list);
    ESP_LOGI(TAG, "→ APs visibles (%d):", ap_count);
    bool found = false;
    for (uint16_t i = 0; i < ap_count; i++) {
        char *auth = "OPEN";
        switch (ap_list[i].authmode) {
            case WIFI_AUTH_OPEN: auth = "OPEN"; break;
            case WIFI_AUTH_WEP: auth = "WEP"; break;
            case WIFI_AUTH_WPA_PSK: auth = "WPA_PSK"; break;
            case WIFI_AUTH_WPA2_PSK: auth = "WPA2_PSK"; break;
            case WIFI_AUTH_WPA_WPA2_PSK: auth = "WPA_WPA2_PSK"; break;
            case WIFI_AUTH_WPA2_ENTERPRISE: auth = "WPA2_ENTERPRISE"; break;
            case WIFI_AUTH_WPA3_PSK: auth = "WPA3_PSK"; break;
            default: break;
        }
        ESP_LOGI(TAG, "  - SSID='%s' CH=%d RSSI=%d AUTH=%s", ap_list[i].ssid, ap_list[i].primary, ap_list[i].rssi, auth);
        if (expected_ssid && strncmp((const char *)ap_list[i].ssid, expected_ssid, sizeof(ap_list[i].ssid)) == 0) {
            found = true;
        }
    }

    if (expected_ssid) {
        if (found) {
            ESP_LOGI(TAG, "✓ SSID objetivo '%s' visible (ok)", expected_ssid);
        } else {
            ESP_LOGW(TAG, "✗ SSID objetivo '%s' NO VISIBLE (posible error: SSID incorrecto, AP apagado, 5GHz-only o 'hidden')", expected_ssid);
        }
    }

    free(ap_list);
}

static const char *wifi_reason_to_str(int reason)
{
    switch (reason) {
        case 1: return "Unspecified/Unknow"; // RFC 802.11 reason 1
        case 2: return "Previous authentication no longer valid (auth expired)";
        case 3: return "Station is leaving (deauth leaving)";
        case 4: return "Disassociated due to inactivity";
        case 5: return "Class 2 frame received from non authenticated STA";
        case 6: return "Class 3 frame received from non associated STA";
        case 8: return "Disassociated because AP is unable to handle all currently associated STAs";
        case 15: return "QoS related reason (rejected)";
        case 32: return "4-Way Handshake timeout";
        case 34: return "Invalid PMKID";
        case 205: return "AP rejected due to security/handshake mismatch (vendor specific)";
        default: return "Unknown reason";
    }
}

static void delayed_connect_task(void *arg)
{
    (void)arg;
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_wifi_connect();
    vTaskDelete(NULL);
}