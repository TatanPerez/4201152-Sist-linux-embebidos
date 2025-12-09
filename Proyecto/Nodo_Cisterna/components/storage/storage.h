#pragma once
#include "esp_err.h"

esp_err_t storage_init(void);
esp_err_t storage_save_float(const char *key, float value);
esp_err_t storage_load_float(const char *key, float *value);
