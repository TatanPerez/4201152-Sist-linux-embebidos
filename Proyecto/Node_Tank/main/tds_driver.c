#include "tds_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <math.h>
#include <string.h>

#define TDS_SAMPLES 16
#define ADC_MAX_MV 3300
#define ADC_MAX_RAW 4095

static const char *TAG = "tds";
static const char *NVS_NAMESPACE = "tds_cal";
static const char *KEY_OFFSET = "tds_offset";
static const char *KEY_GAIN = "tds_gain";

static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_channel_t s_tds_channel = ADC_CHANNEL_0;
static bool s_tds_inited = false;
static float s_tds_offset = 0.0f;
static float s_tds_gain = 1.0f;
static float s_last_raw = 0.0f;

static esp_err_t tds_storage_save_float(const char *key, float value)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_set_blob(handle, key, &value, sizeof(value));
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}

static esp_err_t tds_storage_load_float(const char *key, float *out)
{
    size_t required = sizeof(float);
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_get_blob(handle, key, out, &required);
    nvs_close(handle);
    return err;
}

esp_err_t tds_driver_init(adc_channel_t channel)
{
    s_tds_channel = channel;

    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    esp_err_t err = adc_oneshot_new_unit(&unit_cfg, &s_adc_handle);
    if (err != ESP_OK) {
        return err;
    }

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    esp_err_t cfg_err = adc_oneshot_config_channel(s_adc_handle, s_tds_channel, &chan_cfg);
    if (cfg_err == ESP_OK) {
        s_tds_inited = true;
        tds_load_calibration();
    }
    return cfg_err;
}

float tds_driver_read_raw(void)
{
    if (!s_adc_handle || !s_tds_inited) {
        return -1.0f;
    }
    uint32_t adc_sum = 0;
    for (int i = 0; i < TDS_SAMPLES; i++) {
        int raw = 0;
        if (adc_oneshot_read(s_adc_handle, s_tds_channel, &raw) == ESP_OK) {
            adc_sum += (uint32_t)raw;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    uint32_t raw = adc_sum / TDS_SAMPLES;
    s_last_raw = (float)raw;
    return s_last_raw;
}

float tds_driver_read_ppm(void)
{
    float raw = tds_driver_read_raw();
    if (raw < 0) {
        return raw;
    }

    float normalized = (raw - s_tds_offset) * s_tds_gain;
    float voltage_mv = (normalized * ADC_MAX_MV) / ADC_MAX_RAW;

    /* Simple scaling to ppm-like units; adjust as needed per probe */
    float tds_ppm = voltage_mv; /* mV as base */
    return tds_ppm;
}

void tds_set_calibration_point_A(float raw)
{
    s_tds_offset = raw;
    ESP_LOGI(TAG, "Set calibration A (offset) = %f", s_tds_offset);
}

void tds_set_calibration_point_B(float raw)
{
    if (raw - s_tds_offset == 0.0f) {
        ESP_LOGW(TAG, "Calibration B equals A; gain would be infinite. Ignored.");
        return;
    }
    s_tds_gain = 1.0f / (raw - s_tds_offset);
    ESP_LOGI(TAG, "Set calibration B (gain) = %f (raw=%f)", s_tds_gain, raw);
}

esp_err_t tds_save_calibration(void)
{
    esp_err_t r = tds_storage_save_float(KEY_OFFSET, s_tds_offset);
    if (r != ESP_OK) {
        return r;
    }
    return tds_storage_save_float(KEY_GAIN, s_tds_gain);
}

esp_err_t tds_load_calibration(void)
{
    float offset = 0.0f, gain = 1.0f;
    esp_err_t r1 = tds_storage_load_float(KEY_OFFSET, &offset);
    esp_err_t r2 = tds_storage_load_float(KEY_GAIN, &gain);
    if (r1 == ESP_OK) s_tds_offset = offset;
    if (r2 == ESP_OK) s_tds_gain = gain;

    if (r1 == ESP_OK || r2 == ESP_OK) {
        ESP_LOGI(TAG, "Calibration loaded offset=%f gain=%f", s_tds_offset, s_tds_gain);
        return ESP_OK;
    }
    ESP_LOGI(TAG, "Using default calibration offset=0 gain=1");
    return ESP_FAIL;
}

float tds_get_offset(void) { return s_tds_offset; }
float tds_get_gain(void) { return s_tds_gain; }
