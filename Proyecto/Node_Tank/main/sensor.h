// sensor.h
// Interfaz para leer el nivel de agua. Implementación simple que puede usar un
// sensor ultrasónico (HC-SR04) o un sensor de flotador.

#ifndef SENSOR_H
#define SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

// Inicializa recursos para el sensor (pines, timers, etc).
void sensor_init(void);

// Lee el nivel en centímetros. Retorna valor >=0 si OK, o -1 en caso de error.
float sensor_read_level_cm(void);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_H
