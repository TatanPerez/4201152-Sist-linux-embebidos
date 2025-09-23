#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include "utility.h"

void display_info() {
    size_t total_memory = get_total_memory();
    size_t used_memory  = get_used_memory();
    const char* model   = get_processor_model();
    int num_cores       = get_number_of_cores();
    double load         = get_processor_load();

    clear();
    mvprintw(0, 0, "System Memory and CPU Information");
    mvprintw(2, 0, "Total Memory: %zu MB", total_memory / 1024);
    mvprintw(3, 0, "Used Memory: %zu MB", used_memory / 1024);
    mvprintw(4, 0, "Processor Model: %s", model);
    mvprintw(5, 0, "Number of Cores: %d", num_cores);
    mvprintw(6, 0, "Processor Load: %.2f%%", load);
    refresh();
}

int main() {
    initscr();
    noecho();
    cbreak();

    while (1) {
        display_info();
        sleep(2);
    }

    endwin();
    return 0;
}
