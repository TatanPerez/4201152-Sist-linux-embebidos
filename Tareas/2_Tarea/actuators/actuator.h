#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <stdio.h> // For printf in simulation

/**
 * @brief Polymorphic interface for an actuator.
 *        Allows different actuators (LED, Buzzer, etc.) to be controlled
 *        through a common set of function pointers.
 */
typedef struct actuator_interface {
    void *params; /**< Pointer to actuator-specific parameters (e.g., GPIO pin, state) */

    /**
     * @brief Activates the actuator.
     * @param params Actuator-specific parameters.
     */
    void (*activate)(void *params);

    /**
     * @brief Deactivates the actuator.
     * @param params Actuator-specific parameters.
     */
    void (*deactivate)(void *params);

    /**
     * @brief Gets the current status of the actuator.
     * @param params Actuator-specific parameters.
     * @return An integer representing the status (e.g., 0 for off, 1 for on).
     */
    int (*status)(void *params);

} actuator_interface_t;

#endif // ACTUATOR_H