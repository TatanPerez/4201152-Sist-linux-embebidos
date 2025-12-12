#pragma once

#include "esp_err.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "mqtt_client.h"

typedef struct {
    EventGroupHandle_t wifi_event_group;
    esp_netif_t *netif_ap;
    esp_netif_t *netif_sta;
    esp_mqtt_client_handle_t mqtt_client;
} net_manager_context_t;

/* Init Wi-Fi (AP+STA) and start MQTT broker connection.
 * NVS must be initialized before calling.
 */
esp_err_t net_manager_start(net_manager_context_t *ctx, esp_event_handler_t mqtt_handler, void *handler_ctx);
