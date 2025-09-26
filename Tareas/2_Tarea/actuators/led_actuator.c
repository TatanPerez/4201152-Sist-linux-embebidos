#include "actuator.h"
#include <stdio.h> // For printf
#include <stdbool.h> // For bool

// LED-specific parameters (for simulation, just a state)
typedef struct {
    bool is_on;
} led_params_t;

static void led_activate(void *params) {
    led_params_t *led = (led_params_t *)params;
    if (led != NULL && !led->is_on) {
        led->is_on = true;
        printf("[LED] Activated.\n");
        // In a real embedded system: write HIGH to LED GPIO pin
    }
}

static void led_deactivate(void *params) {
    led_params_t *led = (led_params_t *)params;
    if (led != NULL && led->is_on) {
        led->is_on = false;
        printf("[LED] Deactivated.\n");
        // In a real embedded system: write LOW to LED GPIO pin
    }
}

static int led_status(void *params) {
    led_params_t *led = (led_params_t *)params;
    return (led != NULL && led->is_on) ? 1 : 0;
}

// Define the LED actuator interface instance
// This needs to be declared as 'extern' in controller/ctl.c to be used.
static led_params_t global_led_params = { .is_on = false };

actuator_interface_t led_actuator = {
    .params = &global_led_params,
    .activate = led_activate,
    .deactivate = led_deactivate,
    .status = led_status
};