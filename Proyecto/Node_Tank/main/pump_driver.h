#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"

esp_err_t pump_driver_init(gpio_num_t pin);
void pump_driver_set_state(bool on);
bool pump_driver_get_state(void);
