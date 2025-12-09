#pragma once
#include <stdbool.h>
#include "esp_err.h"

void tds_init(void);
/**
 * Return averaged raw ADC reading (hardware units)
 */
float tds_read_raw(void);

/** Return TDS in ppm (relative) using offset/gain calibration. */
float tds_read_ppm(void);

void tds_set_calibration_point_A(float raw);
void tds_set_calibration_point_B(float raw);
esp_err_t tds_save_calibration(void);
esp_err_t tds_load_calibration(void);

float tds_get_offset(void);
float tds_get_gain(void);
