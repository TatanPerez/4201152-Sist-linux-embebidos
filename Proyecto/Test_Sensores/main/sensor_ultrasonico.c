// // sensor_ultrasonico.c
// // Implementación del sensor HC-SR04 usando GPIO y esp_timer para medir pulsos

// #include "sensor_ultrasonico.h"
// #include <stdio.h>
// #include "esp_log.h"
// #include "driver/gpio.h"
// #include "esp_timer.h"

// static const char *TAG = "HCSR04";

// void sensor_ultrasonico_init(void)
// {
//     // Configura TRIG como salida
//     gpio_config_t io_conf = {};
//     io_conf.intr_type = GPIO_INTR_DISABLE;
//     io_conf.mode = GPIO_MODE_OUTPUT;
//     io_conf.pin_bit_mask = (1ULL << HCSR04_TRIG_GPIO);
//     io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
//     io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
//     gpio_config(&io_conf);

//     // Configura ECHO como entrada
//     io_conf = (gpio_config_t){};
//     io_conf.intr_type = GPIO_INTR_DISABLE;
//     io_conf.mode = GPIO_MODE_INPUT;
//     io_conf.pin_bit_mask = (1ULL << HCSR04_ECHO_GPIO);
//     io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
//     io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
//     gpio_config(&io_conf);

//     // Asegura TRIG en bajo
//     gpio_set_level(HCSR04_TRIG_GPIO, 0);
//     ESP_LOGI(TAG, "Sensor ultrasónico inicializado (TRIG=%d, ECHO=%d)", HCSR04_TRIG_GPIO, HCSR04_ECHO_GPIO);
// }

// int medir_distancia(void)
// {
//     const int timeout_us = 30000; // 30 ms timeout para evitar bloqueos

//     // Genera pulso de 10us en TRIG
//     gpio_set_level(HCSR04_TRIG_GPIO, 0);
//     esp_rom_delay_us(2);
//     gpio_set_level(HCSR04_TRIG_GPIO, 1);
//     esp_rom_delay_us(10);
//     gpio_set_level(HCSR04_TRIG_GPIO, 0);

//     // Espera a que ECHO suba
//     int64_t start_wait = esp_timer_get_time();
//     while (gpio_get_level(HCSR04_ECHO_GPIO) == 0) {
//         if ((esp_timer_get_time() - start_wait) > timeout_us) {
//             ESP_LOGW(TAG, "Timeout esperando ECHO alto");
//             return -1;
//         }
//     }

//     // Marca inicio del pulso
//     int64_t t_start = esp_timer_get_time();

//     // Espera a que ECHO baje
//     while (gpio_get_level(HCSR04_ECHO_GPIO) == 1) {
//         if ((esp_timer_get_time() - t_start) > timeout_us) {
//             ESP_LOGW(TAG, "Timeout esperando ECHO bajo");
//             return -1;
//         }
//     }

//     int64_t t_end = esp_timer_get_time();
//     int64_t pulse_us = t_end - t_start;

//     // Conversión a distancia en cm (velocidad del sonido ~ 34300 cm/s)
//     // Fórmula aproximada: distance_cm = pulse_us / 58
//     int distance_cm = (int)((pulse_us) / 58);

//     ESP_LOGD(TAG, "Pulse: %lld us, Distance: %d cm", pulse_us, distance_cm);
//     return distance_cm;
// }

// sensor_ultrasonico_test.c
#include "sensor_ultrasonico.h"
#include "esp_log.h"
#include <stdio.h>
#include "driver/gpio.h"
#include "esp_timer.h"

// Para FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "HCSR04";

#define DISTANCIA_ESPERADA_CM 50
#define TOLERANCIA_CM 5

void test_sensor_ultrasonico(void)
{
    ESP_LOGI(TAG, "Iniciando test del sensor ultrasónico...");

    // Inicializa el sensor
    sensor_ultrasonico_init();

    // Realiza varias mediciones
    for (int i = 0; i < 5; i++) {
        int distancia = medir_distancia();

        if (distancia < 0) {
            ESP_LOGE(TAG, "Lectura fallida (timeout o error de sensor)");
            printf("Test %d: FAIL\n", i + 1);
        } else {
            printf("Test %d: Distancia medida = %d cm, Esperado = %d cm\n",
                   i + 1, distancia, DISTANCIA_ESPERADA_CM);

            if (distancia >= DISTANCIA_ESPERADA_CM - TOLERANCIA_CM &&
                distancia <= DISTANCIA_ESPERADA_CM + TOLERANCIA_CM) {
                printf("Resultado: PASS\n");
            } else {
                printf("Resultado: FAIL\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // espera 1 segundo entre mediciones
    }

    ESP_LOGI(TAG, "Test del sensor ultrasónico finalizado.");
}
