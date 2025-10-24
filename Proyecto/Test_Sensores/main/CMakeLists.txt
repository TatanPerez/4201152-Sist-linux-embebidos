# idf_component_register(
#     SRCS "main.c"
#          "sensor_ultrasonico.c"
#          "sensor_turbidez.c"
#          "../test/test_sensores.c"
#     INCLUDE_DIRS "." "../test"
# )
idf_component_register(
    SRCS "main.c"
         "sensor_ultrasonico.c"
         "sensor_turbidez.c"
         "test/test_sensores.c"
    INCLUDE_DIRS "." "test"
)
