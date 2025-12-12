#ifndef TASKS_H
#define TASKS_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "../sensors/sensor.h"

/**
 * @brief Estructura para compartir datos entre tareas de forma sincronizada
 */
typedef struct {
    sensor_data_t sensor_data;
    SemaphoreHandle_t mutex;     // Semáforo para sincronizar acceso
} shared_sensor_data_t;

/**
 * @brief Estructura para configuración de tareas
 */
typedef struct {
    uint32_t sampling_interval_ms;  // Intervalo de muestreo (1000ms = 1 segundo)
    int ultrasonic_trig_pin;        // Pin GPIO del sensor ultrasónico TRIG
    int ultrasonic_echo_pin;        // Pin GPIO del sensor ultrasónico ECHO
    int tds_adc_pin;                // Pin ADC del sensor TDS
    int pump_relay_pin;             // Pin GPIO del relé que controla la bomba
    /* pump_button_pin removed: button is disabled in firmware; control via MQTT only */
    /* Note: automatic thresholds removed; control is performed externally via MQTT ON/OFF */
} task_config_t;

/**
 * @brief Inicializa el sistema de tareas FreeRTOS
 * 
 * @param config Estructura con configuración de tareas
 * @return esp_err_t ESP_OK si es exitoso
 */
esp_err_t tasks_init(const task_config_t *config);

/**
 * @brief Obtiene la estructura compartida de datos de sensores
 * 
 * @return shared_sensor_data_t* Puntero a estructura de datos compartidos
 */
shared_sensor_data_t* tasks_get_shared_sensor_data(void);

/**
 * @brief Lee datos de sensores de forma segura (con semáforo)
 * 
 * @param data Puntero para almacenar los datos leídos
 * @param timeout_ms Timeout en ms para adquirir el semáforo
 * @return esp_err_t ESP_OK si es exitoso, ESP_ERR_TIMEOUT si agota tiempo
 */
esp_err_t tasks_read_sensor_data(sensor_data_t *data, uint32_t timeout_ms);

/**
 * @brief Controla el relé de la bomba sumergible
 * 
 * @param enable true para encender, false para apagar
 * @return esp_err_t ESP_OK si es exitoso
 */
esp_err_t tasks_set_pump_relay(bool enable);

/**
 * @brief Obtiene el estado actual del relé de la bomba
 * 
 * @return bool true si está encendido, false si está apagado
 */
bool tasks_get_pump_relay_state(void);

/* Button-related API removed: button is disabled and firmware control is MQTT-only */
typedef void (*pump_state_cb_t)(bool state);
/**
 * @brief Registrar una función callback que será llamada cuando cambie el estado del relé
 */
void tasks_register_pump_state_cb(pump_state_cb_t cb);

#endif // TASKS_H
