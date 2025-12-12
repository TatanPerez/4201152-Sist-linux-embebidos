// sensor_ultrasonico.h
// Prototipos y definiciones para el sensor HC-SR04

#ifndef SENSOR_ULTRASONICO_H
#define SENSOR_ULTRASONICO_H

#ifdef __cplusplus
extern "C" {
#endif

// Pines usados por el HC-SR04
#define HCSR04_TRIG_GPIO 2
#define HCSR04_ECHO_GPIO 3

// Inicializa los pines y recursos necesarios para el sensor ultrasónico
void sensor_ultrasonico_init(void);

// Mide la distancia y devuelve un entero con la distancia en centímetros.
// Retorna -1 en caso de timeout/error.
int medir_distancia(void);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_ULTRASONICO_H
