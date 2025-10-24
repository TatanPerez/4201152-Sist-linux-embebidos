// sensor_turbidez.c
// Lectura ADC para sensor de turbidez (compatible con ESP32-C6)

#include "sensor_turbidez.h"
#include <stdio.h>
#include "esp_log.h"
#include "driver/adc.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG = "TURBIDEZ";

// Manejador de calibración ADC
static adc_cali_handle_t adc_cali_handle = NULL;

// Valor de referencia (por si no hay eFuse)
#define DEFAULT_VREF 1100

void sensor_turbidez_init(void)
{
    // Configura resolución y atenuación
    ESP_ERROR_CHECK(adc1_config_width(ADC_BITWIDTH_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(TURBIDEZ_ADC_CHANNEL, ADC_ATTEN_DB_12));

    // Configura la calibración usando curve fitting
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Calibración ADC habilitada correctamente");
    }
    else
    {
        adc_cali_handle = NULL;
        ESP_LOGW(TAG, "No se pudo habilitar la calibración ADC (usará valor raw)");
    }
}

float leer_turbidez(void)
{
    int raw = adc1_get_raw(TURBIDEZ_ADC_CHANNEL);
    int voltage_mv = 0;

    if (adc_cali_handle)
    {
        adc_cali_raw_to_voltage(adc_cali_handle, raw, &voltage_mv);
    }
    else
    {
        voltage_mv = (raw * 3300) / 4095;
    }


    float voltage_v = voltage_mv / 1000.0f;
    ESP_LOGD(TAG, "ADC raw=%d, volt=%dmV (%.3f V)", raw, voltage_mv, voltage_v);
    return voltage_v;
}
