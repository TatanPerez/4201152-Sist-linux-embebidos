#include "actuator.h"
#include <stdio.h> // For printf
#include <stdbool.h> // For bool

// Buzzer-specific parameters (for simulation, just a state)
typedef struct {
    bool is_on;
} buzzer_params_t;

static void buzzer_activate(void *params) {
    buzzer_params_t *buzzer = (buzzer_params_t *)params;
    if (buzzer != NULL && !buzzer->is_on) {
        buzzer->is_on = true;
        printf("[BUZZER] Activated.\n");
        // In a real embedded system: enable buzzer output
    }
}

static void buzzer_deactivate(void *params) {
    buzzer_params_t *buzzer = (buzzer_params_t *)params;
    if (buzzer != NULL && buzzer->is_on) {
        buzzer->is_on = false;
        printf("[BUZZER] Deactivated.\n");
        // In a real embedded system: disable buzzer output
    }
}

static int buzzer_status(void *params) {
    buzzer_params_t *buzzer = (buzzer_params_t *)params;
    return (buzzer != NULL && buzzer->is_on) ? 1 : 0;
}

// Define the Buzzer actuator interface instance
// This needs to be declared as 'extern' in controller/ctl.c to be used.
static buzzer_params_t global_buzzer_params = { .is_on = false };

actuator_interface_t buzzer_actuator = {
    .params = &global_buzzer_params,
    .activate = buzzer_activate,
    .deactivate = buzzer_deactivate,
    .status = buzzer_status
};