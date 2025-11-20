// tasks.h
// Declaración de tareas y utilidades para sincronización.

#ifndef TASKS_H
#define TASKS_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

// Inicia las tareas del nodo. `mutex` se usa para proteger accesos compartidos
// como el sensor y la publicación MQTT.
void tasks_start(SemaphoreHandle_t mutex);

#ifdef __cplusplus
}
#endif

#endif // TASKS_H
