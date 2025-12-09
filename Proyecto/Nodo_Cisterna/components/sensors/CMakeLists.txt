# CMakeLists.txt para componente Sensores

idf_component_register(SRCS "sensor.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_adc esp_timer)
