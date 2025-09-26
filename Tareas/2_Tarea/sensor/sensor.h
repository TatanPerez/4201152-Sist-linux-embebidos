
#ifndef SENSOR_H
#define SENSOR_H

/**
 * @brief Initializes the sensor module.
 *        This might involve setting up hardware or opening data sources.
 */
void sensor_init(void);

/**
 * @brief Reads a value from the sensor.
 * @return The sensor reading as a double.
 */
double sensor_read(void);

#endif // SENSOR_H