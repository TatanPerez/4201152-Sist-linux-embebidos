idf_component_register(
    SRCS "net_manager.c" "softap_sta.c" "pump_driver.c" "ultrasonic_driver.c" "tds_driver.c"
    PRIV_REQUIRES esp_wifi nvs_flash esp_netif esp_event mqtt esp_adc esp_driver_gpio
    INCLUDE_DIRS "."
)
