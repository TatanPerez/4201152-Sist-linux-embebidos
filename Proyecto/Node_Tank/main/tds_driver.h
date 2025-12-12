#pragma once

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"

esp_err_t tds_driver_init(adc_channel_t channel);
float tds_driver_read_raw(void);
float tds_driver_read_ppm(void);

/* Calibration helpers */
void tds_set_calibration_point_A(float raw);
void tds_set_calibration_point_B(float raw);
esp_err_t tds_save_calibration(void);
esp_err_t tds_load_calibration(void);
float tds_get_offset(void);
float tds_get_gain(void);
