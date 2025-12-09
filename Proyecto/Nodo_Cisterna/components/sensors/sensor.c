#include <string.h>
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_timer.h"

#include "sensor.h"

static const char *TAG = "SENSOR";

// Variables para el sensor ultrasónico
static int g_trig_pin = -1;
static int g_echo_pin = -1;
static adc_oneshot_unit_handle_t adc_handle = NULL;
static int g_tds_adc_channel = -1;

// Constantes para sensor ultrasónico
#define ULTRASONIC_PULSE_DURATION_US 10

/**
 * @brief Inicializa los sensores (ultrasónico y TDS)
 */
esp_err_t sensor_init(int ultrasonic_trig_pin, int ultrasonic_echo_pin, 
                      int tds_adc_pin)
{
    ESP_LOGI(TAG, "→ Inicializando sensores...");

    // Almacenar pines
    g_trig_pin = ultrasonic_trig_pin;
    g_echo_pin = ultrasonic_echo_pin;
    g_tds_adc_channel = tds_adc_pin;

    // ========== Configurar sensor ultrasónico ==========
    // Configurar TRIG como salida
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << g_trig_pin),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error configurando TRIG: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configurar ECHO como entrada
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << g_echo_pin);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    
    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error configurando ECHO: %s", esp_err_to_name(ret));
        return ret;
    }

    // ========== Configurar sensor TDS (ADC) ==========
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ret = adc_oneshot_new_unit(&init_config, &adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error inicializando ADC: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configurar canal ADC para TDS
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,  // Rango 0-3600mV
    };

    ret = adc_oneshot_config_channel(adc_handle, (adc_channel_t)g_tds_adc_channel, 
                                      &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error configurando canal ADC: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "✓ Sensores inicializados correctamente");
    return ESP_OK;
}

/**
 * @brief Lee el nivel de agua mediante sensor ultrasónico
 * 
 * Calcula la distancia basada en:
 * - Envía pulso de 10µs al pin TRIG
 * - Mide tiempo del pulso ECHO
 * - Distancia = (tiempo_echo * velocidad_sonido) / 2
 */
esp_err_t sensor_read_ultrasonic(float *distance)
{
    if (distance == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (g_trig_pin < 0 || g_echo_pin < 0) {
        ESP_LOGE(TAG, "✗ Sensor ultrasónico no inicializado");
        return ESP_ERR_INVALID_STATE;
    }

    // Enviar pulso de 10µs
    gpio_set_level(g_trig_pin, 0);
    esp_rom_delay_us(2);
    gpio_set_level(g_trig_pin, 1);
    esp_rom_delay_us(ULTRASONIC_PULSE_DURATION_US);
    gpio_set_level(g_trig_pin, 0);

    // Esperar a que ECHO se ponga en alto
    // Aumentamos el timeout inicial a 30000 µs (30 ms) para evitar falsos timeouts
    uint32_t timeout = 30000;  // 30ms timeout
    uint32_t start_time = esp_timer_get_time();
    while (gpio_get_level(g_echo_pin) == 0) {
        if ((esp_timer_get_time() - start_time) > timeout) {
            ESP_LOGW(TAG, "✗ Timeout esperando ECHO alto");
            *distance = 0.0f;
            return ESP_ERR_TIMEOUT;
        }
    }

    // Medir duración del pulso ECHO
    uint32_t echo_start = esp_timer_get_time();
    timeout = 100000;  // 100ms timeout (máximo ~17 metros)
    while (gpio_get_level(g_echo_pin) == 1) {
        if ((esp_timer_get_time() - echo_start) > timeout) {
            ESP_LOGW(TAG, "✗ Timeout esperando ECHO bajo");
            *distance = 0.0f;
            return ESP_ERR_TIMEOUT;
        }
    }
    uint32_t echo_duration = esp_timer_get_time() - echo_start;

    // Calcular distancia: distancia = (tiempo * velocidad_sonido) / 2
    // Velocidad del sonido = 343 m/s = 0.0343 cm/µs
    // Dividimos por 2 porque el sonido viaja ida y vuelta
    *distance = (echo_duration * 0.0343f) / 2.0f;

    return ESP_OK;
}

/**
 * @brief Lee el valor TDS mediante sensor analógico
 * 
 * Convierte el valor ADC a ppm:
 * - Rango ADC: 0-4095
 * - Voltaje máximo: 3.3V
 * - Voltaje = (valor_ADC / 4095) * 3.3
 * - TDS (ppm) = (Voltaje - 0.05) / 0.065
 */
esp_err_t sensor_read_tds(float *tds_value)
{
    if (tds_value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (adc_handle == NULL) {
        ESP_LOGE(TAG, "✗ Sensor TDS (ADC) no inicializado");
        return ESP_ERR_INVALID_STATE;
    }

    int adc_reading = 0;
    esp_err_t ret = adc_oneshot_read(adc_handle, (adc_channel_t)g_tds_adc_channel, 
                                     &adc_reading);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ Error leyendo ADC: %s", esp_err_to_name(ret));
        return ret;
    }

    // Convertir valor ADC a voltaje (3.3V rango)
    float voltage = (adc_reading / 4095.0f) * 3.3f;

    // Convertir voltaje a ppm usando calibración estándar
    // Fórmula: ppm = (Voltaje - 0.05) / 0.065
    *tds_value = (voltage - 0.05f) / 0.065f;

    // Limitar a rango razonable
    if (*tds_value < 0.0f) {
        *tds_value = 0.0f;
    }

    return ESP_OK;
}

/**
 * @brief Clasifica la calidad del agua según el valor de TDS
 * 
 * Clasificación:
 * - < 300 ppm: agua limpia (WATER_STATE_CLEAN)
 * - 300-600 ppm: agua en estado medio (WATER_STATE_MEDIUM)
 * - > 600 ppm: agua sucia (WATER_STATE_DIRTY)
 */
water_state_t sensor_classify_water_quality(float tds_value)
{
    if (tds_value < 300.0f) {
        return WATER_STATE_CLEAN;
    } else if (tds_value <= 600.0f) {
        return WATER_STATE_MEDIUM;
    } else {
        return WATER_STATE_DIRTY;
    }
}

/**
 * @brief Lee ambos sensores y devuelve estructura completa de datos
 */
esp_err_t sensor_read_all(sensor_data_t *data)
{
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Limpiar estructura
    memset(data, 0, sizeof(sensor_data_t));

    // Obtener timestamp
    data->timestamp = (uint32_t)(esp_timer_get_time() / 1000000);

    // Leer sensor ultrasónico
    esp_err_t ret = sensor_read_ultrasonic(&data->water_level);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "✗ Error leyendo sensor ultrasónico");
        data->water_level = -1.0f;
    }

    // Leer sensor TDS
    ret = sensor_read_tds(&data->tds_value);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "✗ Error leyendo sensor TDS");
        data->tds_value = -1.0f;
    }

    // Clasificar calidad del agua
    if (data->tds_value >= 0.0f) {
        data->water_state = sensor_classify_water_quality(data->tds_value);
    } else {
        data->water_state = WATER_STATE_CLEAN;
    }

    return ESP_OK;
}
