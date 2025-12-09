#ifndef SENSOR_H
#define SENSOR_H

#include "esp_err.h"

/**
 * @brief Estados de clasificación de calidad del agua según TDS
 */
typedef enum {
    WATER_STATE_CLEAN = 0,      // < 300 ppm
    WATER_STATE_MEDIUM = 1,     // 300-600 ppm
    WATER_STATE_DIRTY = 2       // > 600 ppm
} water_state_t;

/**
 * @brief Estructura para almacenar datos de sensores
 */
typedef struct {
    float water_level;           // Nivel de agua en cm
    float tds_value;             // Valor de TDS en ppm
    water_state_t water_state;   // Estado del agua (limpia, media, sucia)
    uint32_t timestamp;          // Timestamp de la lectura
} sensor_data_t;

/**
 * @brief Inicializa los sensores (ultrasónico y TDS)
 * 
 * Configura los pines GPIO necesarios para:
 * - Sensor ultrasónico (TRIG y ECHO)
 * - Sensor TDS (ADC)
 * 
 * @param ultrasonic_trig_pin Pin GPIO para TRIG del sensor ultrasónico
 * @param ultrasonic_echo_pin Pin GPIO para ECHO del sensor ultrasónico
 * @param tds_adc_pin Pin ADC para el sensor TDS
 * @return esp_err_t ESP_OK si es exitoso
 */
esp_err_t sensor_init(int ultrasonic_trig_pin, int ultrasonic_echo_pin, int tds_adc_pin);

/**
 * @brief Lee el nivel de agua mediante sensor ultrasónico
 * 
 * @param distance Puntero para almacenar la distancia en cm
 * @return esp_err_t ESP_OK si es exitoso
 */
esp_err_t sensor_read_ultrasonic(float *distance);

/**
 * @brief Lee el valor TDS mediante sensor analógico
 * 
 * @param tds_value Puntero para almacenar el valor TDS en ppm
 * @return esp_err_t ESP_OK si es exitoso
 */
esp_err_t sensor_read_tds(float *tds_value);

/**
 * @brief Clasifica la calidad del agua según el valor de TDS
 * 
 * Clasificación:
 * - < 300 ppm: agua limpia
 * - 300-600 ppm: agua en estado medio
 * - > 600 ppm: agua sucia
 * 
 * @param tds_value Valor de TDS en ppm
 * @return water_state_t Estado clasificado del agua
 */
water_state_t sensor_classify_water_quality(float tds_value);

/**
 * @brief Lee ambos sensores y devuelve estructura completa de datos
 * 
 * @param data Puntero a estructura para almacenar los datos leídos
 * @return esp_err_t ESP_OK si es exitoso
 */
esp_err_t sensor_read_all(sensor_data_t *data);

/* Simple programmatic command API so external tasks (UART handler) can invoke
    calibration without using esp_console/argtable which has caused instability. */
void sensor_do_calA(void);
void sensor_do_calB(void);
void sensor_do_save(void);
void sensor_do_show(void);

#endif // SENSOR_H
