
#include "sensor.h"
#include <stdio.h>
#include <stdlib.h> // For exit()
#include <string.h> // For strncmp

// Path to the sensor data file
#define SENSOR_FEED_PATH "tests/sensor_feed.csv"
#define MAX_LINE_LENGTH 128

static FILE *sensor_feed_file = NULL;

void sensor_init(void) {
    if (sensor_feed_file != NULL) {
        printf("Sensor already initialized.\n");
        return;
    }

    sensor_feed_file = fopen(SENSOR_FEED_PATH, "r");
    if (sensor_feed_file == NULL) {
        perror("Error opening sensor_feed.csv");
        fprintf(stderr, "Please ensure '%s' exists in the 'tests' directory relative to the executable.\n", SENSOR_FEED_PATH);
        exit(EXIT_FAILURE);
    }
    printf("Sensor initialized. Reading from '%s'.\n", SENSOR_FEED_PATH);
}

double sensor_read(void) {
    if (sensor_feed_file == NULL) {
        fprintf(stderr, "Sensor not initialized. Call sensor_init() first.\n");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    double value;

    if (fgets(line, sizeof(line), sensor_feed_file) != NULL) {
        if (sscanf(line, "%lf", &value) == 1) {
            return value;
        } else {
            fprintf(stderr, "Warning: Could not parse sensor value from line: %s", line);
        }
    }

    // If EOF or parsing failed, reset to beginning of file and try again
    if (feof(sensor_feed_file)) {
        printf("End of sensor feed reached. Looping back to start.\n");
        rewind(sensor_feed_file); // Go back to the beginning of the file
        // Try to read the first line after rewind
        if (fgets(line, sizeof(line), sensor_feed_file) != NULL) {
            if (sscanf(line, "%lf", &value) == 1) {
                return value;
            }
        }
    }

    // Fallback if something went seriously wrong or file is empty
    fprintf(stderr, "Error: Could not read any valid sensor value. Returning 0.0.\n");
    return 0.0;
}

// Optional: Add a cleanup function
void sensor_deinit(void) {
    if (sensor_feed_file != NULL) {
        fclose(sensor_feed_file);
        sensor_feed_file = NULL;
        printf("Sensor de-initialized.\n");
    }
}