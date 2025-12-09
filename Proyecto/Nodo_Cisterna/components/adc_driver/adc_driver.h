#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

esp_err_t adc_init(int channel);
/**
 * Read averaged raw ADC value (0..4095 or hardware-dependent).
 * samples: number of samples to average
 */
int adc_read_raw(int samples);

/** Read measured voltage in millivolts (averaged). samples: number of samples */
float adc_read_voltage(int samples);
