#ifndef UTILITY_H
#define UTILITY_H

#include <stddef.h>

size_t get_total_memory();       // en KB
size_t get_used_memory();        // en KB
const char* get_processor_model();
int get_number_of_cores();
double get_processor_load();     // en %

#endif // UTILITY_H
