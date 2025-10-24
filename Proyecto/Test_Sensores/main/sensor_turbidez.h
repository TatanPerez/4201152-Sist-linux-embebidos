#ifndef SENSOR_TURBIDEZ_H
#define SENSOR_TURBIDEZ_H

#ifdef __cplusplus
extern "C" {
#endif

#define TURBIDEZ_ADC_CHANNEL ADC_CHANNEL_0  // o ADC1_CHANNEL_0 según placa

void sensor_turbidez_init(void);
float leer_turbidez(void);

#ifdef __cplusplus
}
#endif

#endif
