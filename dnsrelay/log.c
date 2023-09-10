#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static struct timespec start_time;

int __log_level;

void log_init(void) {
    timespec_get(&start_time, TIME_UTC);
}

void log_set_level(int level) {
    __log_level = level;
}

double __log_time(void) {
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    return (now.tv_sec - start_time.tv_sec) + (now.tv_nsec - start_time.tv_nsec) * 1e-9;
}
