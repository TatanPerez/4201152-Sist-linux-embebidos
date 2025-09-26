// Define la fuente POSIX para habilitar clock_gettime y otras funciones POSIX.
// DEBE estar antes de cualquier #include.
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>    // Para clock_gettime, struct timespec
#include <unistd.h>  // Para usleep (simulando delay)
#include <signal.h>  // Para manejo de señales

#include "sensor.h"
#include "actuator.h" // Incluye la definición de actuator_interface_t

// Declaraciones anticipadas para las instancias de actuadores definidas en sus respectivos archivos .c
extern actuator_interface_t led_actuator;
extern actuator_interface_t buzzer_actuator;

// --- Configuración ---
#define SAMPLE_INTERVAL_MS 100 // Muestrear sensor cada 100 ms
#define THRESHOLD          50.0 // Umbral del valor del sensor
#define BUZZER_OFF_DELAY_S 1    // Apagar buzzer 1 segundo después de que el valor < umbral
#define LED_OFF_DELAY_S    5    // Apagar LED 5 segundos después de que el valor < umbral

// --- Variables globales para terminación ---
static volatile sig_atomic_t running = 1;

// Manejador de señales para un apagado ordenado
void sigint_handler(int signum) {
    (void)signum; // Le dice al compilador que este parámetro no se usa intencionalmente
    printf("\nReceived SIGINT. Shutting down gracefully...\n");
    running = 0;
}

// Función auxiliar para obtener el tiempo monotónico actual en milisegundos
long long get_monotonic_time_ms() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int main() {
    // Registrar el manejador de señales para Ctrl+C
    signal(SIGINT, sigint_handler);

    printf("Controller started.\n");
    printf("Threshold: %.2f\n", THRESHOLD);
    printf("Sample Interval: %dms\n", SAMPLE_INTERVAL_MS);

    sensor_init();

    // Variables para los temporizadores de desactivación
    long long buzzer_deactivate_time_ms = 0; // 0 significa que no hay desactivación programada
    long long led_deactivate_time_ms = 0;   // 0 significa que no hay desactivación programada

    while (running) {
        long long current_time_ms = get_monotonic_time_ms();
        double sensor_value = sensor_read();

        // Registrar estado actual
        printf("[%lldms] Sensor: %.2f | LED: %s | Buzzer: %s",
               current_time_ms, sensor_value,
               led_actuator.status(led_actuator.params) ? "ON" : "OFF",
               buzzer_actuator.status(buzzer_actuator.params) ? "ON" : "OFF");

        if (sensor_value >= THRESHOLD) {
            // Activar inmediatamente LED y buzzer
            if (!led_actuator.status(led_actuator.params)) {
                led_actuator.activate(led_actuator.params);
            }
            if (!buzzer_actuator.status(buzzer_actuator.params)) {
                buzzer_actuator.activate(buzzer_actuator.params);
            }
            // Cancelar los temporizadores de desactivación
            buzzer_deactivate_time_ms = 0;
            led_deactivate_time_ms = 0;
            printf(" -> Threshold met. Actuators ON.\n");

        } else { // sensor_value < THRESHOLD
            // Programar apagado del buzzer en 1s y del LED en 5s
            if (buzzer_actuator.status(buzzer_actuator.params) && buzzer_deactivate_time_ms == 0) {
                buzzer_deactivate_time_ms = current_time_ms + BUZZER_OFF_DELAY_S * 1000;
                printf(" -> Threshold below. Buzzer scheduled OFF in %ds.\n", BUZZER_OFF_DELAY_S);
            }
            if (led_actuator.status(led_actuator.params) && led_deactivate_time_ms == 0) {
                led_deactivate_time_ms = current_time_ms + LED_OFF_DELAY_S * 1000;
                printf(" -> Threshold below. LED scheduled OFF in %ds.\n", LED_OFF_DELAY_S);
            }

            // Verificar si los tiempos de desactivación han pasado
            if (buzzer_deactivate_time_ms != 0 && current_time_ms >= buzzer_deactivate_time_ms) {
                buzzer_actuator.deactivate(buzzer_actuator.params);
                buzzer_deactivate_time_ms = 0; // Reiniciar temporizador
            }
            if (led_deactivate_time_ms != 0 && current_time_ms >= led_deactivate_time_ms) {
                led_actuator.deactivate(led_actuator.params);
                led_deactivate_time_ms = 0; // Reiniciar temporizador
            }

            if (!buzzer_actuator.status(buzzer_actuator.params) && !led_actuator.status(led_actuator.params)) {
                 printf(" -> Threshold below. Actuators OFF.\n");
            } else {
                 printf(" -> Threshold below. Actuators ON (scheduled off).\n");
            }
        }

        fflush(stdout); // Asegurar que los logs se impriman inmediatamente

        // Simular delay para la siguiente muestra usando el método estándar POSIX
        struct timespec delay = {0};
        delay.tv_sec = SAMPLE_INTERVAL_MS / 1000; // Parte en segundos
        delay.tv_nsec = (SAMPLE_INTERVAL_MS % 1000) * 1000000L; // Parte en nanosegundos
        nanosleep(&delay, NULL);
    }

    // Des-inicializar el sensor (si existe una función deinit)
    // sensor_deinit();

    printf("Controller gracefully shut down.\n");
    return 0;
}