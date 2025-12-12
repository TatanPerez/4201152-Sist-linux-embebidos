#pragma once

#include "esp_err.h"
#include "driver/gpio.h"

esp_err_t ultrasonic_driver_init(gpio_num_t trig_pin, gpio_num_t echo_pin);
float ultrasonic_driver_read_cm(void);
