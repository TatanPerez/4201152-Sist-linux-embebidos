idf_component_register(
    SRCS "main.c"
         "sensor_ultrasonico.c"
         "sensor_turbidez.c"
         "mqtt.c"
         "sensor.c"
         "wifi.c"
         "tasks.c"
    INCLUDE_DIRS "."
)
