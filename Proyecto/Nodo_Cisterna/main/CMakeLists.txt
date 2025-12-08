idf_component_register(SRCS "main.c" "port_compat.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_wifi freertos nvs_flash esp_netif esp_event tasks mqtt_stub wifi sensors)
