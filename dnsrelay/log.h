#pragma once
#include <stdio.h>

#define LOG_LEVEL_SLIENT 0
#define LOG_LEVEL_FATAL 1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_STAT 3
#define LOG_LEVEL_INFO 4
#define LOG_LEVEL_DEBUG 5

#define log_fatal(fmt, ...)                                          \
    if (__log_level >= LOG_LEVEL_FATAL) {                            \
        printf("[%6.3lf] Fatal: " fmt, __log_time(), ##__VA_ARGS__); \
    }

#define log_error(fmt, ...)                                          \
    if (__log_level >= LOG_LEVEL_ERROR) {                            \
        printf("[%6.3lf] Error: " fmt, __log_time(), ##__VA_ARGS__); \
    }

#define log_stat(fmt, ...)                                           \
    if (__log_level >= LOG_LEVEL_STAT) {                             \
        printf("[%6.3lf] Stat:  " fmt, __log_time(), ##__VA_ARGS__); \
    }

#define log_info(fmt, ...)                                           \
    if (__log_level >= LOG_LEVEL_INFO) {                             \
        printf("[%6.3lf] Info:  " fmt, __log_time(), ##__VA_ARGS__); \
    }

#define log_debug(fmt, ...)                                          \
    if (__log_level >= LOG_LEVEL_DEBUG) {                            \
        printf("[%6.3lf] Debug: " fmt, __log_time(), ##__VA_ARGS__); \
    }

void log_init(void);
void log_set_level(int level);
double __log_time(void);

extern int __log_level;
