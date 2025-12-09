#include "tds.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "adc_driver.h"
#include "storage.h"
#include "esp_err.h"

static const char *TAG = "tds";

// Fixed water temperature for temperature compensation (not used deeply here)
const float WATER_TEMP = 25.0f;

// Calibration storage keys
static const char *KEY_OFFSET = "tds_offset";
static const char *KEY_GAIN = "tds_gain";

static float tds_offset = 0.0f;
static float tds_gain = 1.0f;
static float last_raw = 0.0f;

void tds_init(void)
{
    ESP_LOGI(TAG, "Initializing TDS module");
    // Ensure ADC is initialized by caller
    // Load calibration if present
    esp_err_t r = tds_load_calibration();
    if (r != ESP_OK) {
        ESP_LOGI(TAG, "Using default calibration offset=0 gain=1");
    }
}

float tds_read_raw(void)
{
    // Use ADC driver to read raw and return as float
    int raw = adc_read_raw(20);
    last_raw = (float)raw;
    return last_raw;
}

float tds_read_ppm(void)
{
    float raw = tds_read_raw();
    float normalized = (raw - tds_offset) * tds_gain;
    // Temperature compensation could be applied here based on WATER_TEMP
    float tds_ppm = normalized * 1000.0f; // arbitrary scaling to ppm-like units
    return tds_ppm;
}

void tds_set_calibration_point_A(float raw)
{
    tds_offset = raw;
    ESP_LOGI(TAG, "Set calibration A (offset) = %f", tds_offset);
}

void tds_set_calibration_point_B(float raw)
{
    // raw is expected to be different from offset
    if (raw - tds_offset == 0.0f) {
        ESP_LOGW(TAG, "Calibration B equals A; gain would be infinite. Ignored.");
        return;
    }
    tds_gain = 1.0f / (raw - tds_offset);
    ESP_LOGI(TAG, "Set calibration B (gain) = %f (raw=%f)", tds_gain, raw);
}

esp_err_t tds_save_calibration(void)
{
    esp_err_t r = storage_save_float(KEY_OFFSET, tds_offset);
    if (r != ESP_OK) return r;
    r = storage_save_float(KEY_GAIN, tds_gain);
    return r;
}

esp_err_t tds_load_calibration(void)
{
    float offset = 0.0f, gain = 1.0f;
    esp_err_t r1 = storage_load_float(KEY_OFFSET, &offset);
    esp_err_t r2 = storage_load_float(KEY_GAIN, &gain);
    if (r1 == ESP_OK) tds_offset = offset;
    if (r2 == ESP_OK) tds_gain = gain;

    if (r1 == ESP_OK || r2 == ESP_OK) {
        ESP_LOGI(TAG, "Calibration loaded offset=%f gain=%f", tds_offset, tds_gain);
        return ESP_OK;
    }
    return ESP_FAIL;
}

float tds_get_offset(void) { return tds_offset; }
float tds_get_gain(void) { return tds_gain; }
