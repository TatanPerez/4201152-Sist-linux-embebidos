#include "adc_driver.h"

#include <stdio.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

static const char *TAG = "adc_driver";

// ADC configuration - adjust channel/unit as needed for your board
#define ADC_UNIT_ID ADC_UNIT_1
#define ADC_CHANNEL ADC_CHANNEL_0 // change as needed
#define ADC_ATTEN ADC_ATTEN_DB_11
#define DEFAULT_VREF 1100


static adc_oneshot_unit_handle_t adc_handle = NULL;

esp_err_t adc_init(void)
{
    esp_err_t ret = ESP_OK;

    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_ID,
    };
    ret = adc_oneshot_new_unit(&init_cfg, &adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "adc_oneshot_new_unit failed: %s", esp_err_to_name(ret));
        return ret;
    }

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN,
    };
    ret = adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &chan_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "adc_oneshot_config_channel failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Note: esp_adc_cal not used here. Use simple linear conversion from raw to mV below.
    ESP_LOGI(TAG, "ADC initialized (oneshot)");
    return ESP_OK;
}

int adc_read_raw(int samples)
{
    if (samples <= 0) samples = 10;
    long sum = 0;
    for (int i = 0; i < samples; ++i) {
        int raw = 0;
        esp_err_t r = adc_oneshot_read(adc_handle, ADC_CHANNEL, &raw);
        if (r != ESP_OK) {
            ESP_LOGW(TAG, "adc_oneshot_read failed: %s", esp_err_to_name(r));
            continue;
        }
        sum += raw;
    }
    int avg = (int)(sum / samples);
    return avg;
}

float adc_read_voltage(int samples)
{
    if (samples <= 0) samples = 10;
    int raw = adc_read_raw(samples);
    uint32_t voltage = (uint32_t)((raw / 4095.0f) * DEFAULT_VREF);
    return (float)voltage; // millivolts
}
