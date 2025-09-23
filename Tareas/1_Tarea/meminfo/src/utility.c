#include <stdio.h>
#include <stdlib.h>
#include <string.h>   // <- necesario para strstr
#include "utility.h"

size_t get_total_memory() {
    FILE *file = fopen("/proc/meminfo", "r");
    if (!file) {
        perror("Failed to open /proc/meminfo");
        return 0;
    }

    size_t total_memory = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "MemTotal: %zu kB", &total_memory) == 1) {
            break;
        }
    }

    fclose(file);
    return total_memory;
}

size_t get_used_memory() {
    FILE *file = fopen("/proc/meminfo", "r");
    if (!file) {
        perror("Failed to open /proc/meminfo");
        return 0;
    }

    size_t total_memory = 0;
    size_t available_memory = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "MemTotal: %zu kB", &total_memory);
        sscanf(line, "MemAvailable: %zu kB", &available_memory);
    }

    fclose(file);
    return total_memory - available_memory;
}

const char* get_processor_model() {
    FILE *file = fopen("/proc/cpuinfo", "r");
    if (!file) {
        perror("Failed to open /proc/cpuinfo");
        return "Unknown";
    }

    static char model[256];
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "model name\t: %[^\n]", model) == 1) {
            break;
        }
    }

    fclose(file);
    return model;
}

int get_number_of_cores() {
    FILE *file = fopen("/proc/cpuinfo", "r");
    if (!file) {
        perror("Failed to open /proc/cpuinfo");
        return -1;
    }

    int cores = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "processor")) {
            cores++;
        }
    }

    fclose(file);
    return cores;
}

double get_processor_load() {
    FILE *file = fopen("/proc/stat", "r");
    if (!file) {
        perror("Failed to open /proc/stat");
        return 0.0;
    }

    long user, nice, system, idle, iowait, irq, softirq;
    char line[256];
    fgets(line, sizeof(line), file);
    sscanf(line, "cpu %ld %ld %ld %ld %ld %ld %ld", 
           &user, &nice, &system, &idle, &iowait, &irq, &softirq);

    static long prev_total = 0, prev_idle = 0;
    long total = user + nice + system + idle + iowait + irq + softirq;
    long total_diff = total - prev_total;
    long idle_diff = idle - prev_idle;

    prev_total = total;
    prev_idle = idle;

    fclose(file);

    if (total_diff == 0) return 0.0;
    return (double)(total_diff - idle_diff) / total_diff * 100.0;
}
