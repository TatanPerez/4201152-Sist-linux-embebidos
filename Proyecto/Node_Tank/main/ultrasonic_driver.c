#include "ultrasonic_driver.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"
#include "esp_log.h"

static const char *TAG_ULTRA = "ultrasonic";
static gpio_num_t s_trig_pin = GPIO_NUM_NC;
static gpio_num_t s_echo_pin = GPIO_NUM_NC;

esp_err_t ultrasonic_driver_init(gpio_num_t trig_pin, gpio_num_t echo_pin)
{
    s_trig_pin = trig_pin;
    s_echo_pin = echo_pin;

    gpio_config_t ultrasonic_conf = {
        .pin_bit_mask = (1ULL << s_trig_pin) | (1ULL << s_echo_pin),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&ultrasonic_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_ULTRA, "GPIO config failed: %d", err);
        return err;
    }

    gpio_set_direction(s_trig_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(s_echo_pin, GPIO_MODE_INPUT);
    gpio_set_level(s_trig_pin, 0);

    return ESP_OK;
}

float ultrasonic_driver_read_cm(void)
{
    if (s_trig_pin == GPIO_NUM_NC || s_echo_pin == GPIO_NUM_NC) {
        return -1.0f;
    }

    gpio_set_level(s_trig_pin, 0);
    ets_delay_us(2);
    gpio_set_level(s_trig_pin, 1);
    ets_delay_us(10);
    gpio_set_level(s_trig_pin, 0);

    int64_t start_wait = esp_timer_get_time();
    while (gpio_get_level(s_echo_pin) == 0) {
        if (esp_timer_get_time() - start_wait > 30000) {
            return -1.0f;
        }
    }

    int64_t echo_start = esp_timer_get_time();
    while (gpio_get_level(s_echo_pin) == 1) {
        if (esp_timer_get_time() - echo_start > 60000) {
            return -1.0f;
        }
    }
    int64_t echo_end = esp_timer_get_time();

    int64_t pulse_width_us = echo_end - echo_start;
    if (pulse_width_us <= 0) {
        return -1.0f;
    }

    return (float)(pulse_width_us * 0.0343f / 2.0f);
}
