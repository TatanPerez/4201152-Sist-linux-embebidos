#include "pump_driver.h"
#include "esp_log.h"

static const char *TAG_PUMP = "pump_driver";
static gpio_num_t s_pump_pin = GPIO_NUM_NC;
static bool s_pump_state = false;

esp_err_t pump_driver_init(gpio_num_t pin)
{
    s_pump_pin = pin;
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << s_pump_pin,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_PUMP, "GPIO config failed: %d", err);
        return err;
    }
    pump_driver_set_state(false);
    return ESP_OK;
}

void pump_driver_set_state(bool on)
{
    s_pump_state = on;
    if (s_pump_pin != GPIO_NUM_NC) {
        gpio_set_level(s_pump_pin, on ? 1 : 0);
    }
}

bool pump_driver_get_state(void)
{
    return s_pump_state;
}
