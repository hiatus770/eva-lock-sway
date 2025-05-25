#pragma once 


#include <stdio.h>
#include <stdarg.h>
#include <time.h>

// ANSI color codes
#define LOG_COLOR_RESET  "\x1b[0m"
#define LOG_COLOR_RED    "\x1b[31m"
#define LOG_COLOR_GREEN  "\x1b[32m"
#define LOG_COLOR_YELLOW "\x1b[33m"
#define LOG_COLOR_CYAN   "\x1b[36m"

// Internal helper to get current timestamp
static void _log_get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tstruct = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tstruct);
}

// Internal helper to print the log message
static void _log_print(const char *level, const char *color, const char *format, va_list args) {
    char timestamp[20];
    _log_get_timestamp(timestamp, sizeof(timestamp));
    printf("%s[%s] %s: ", color, timestamp, level);
    vprintf(format, args);
    printf("%s\n", LOG_COLOR_RESET);
}

// Public logging functions
static void log_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    _log_print("INFO", LOG_COLOR_GREEN, format, args);
    va_end(args);
}

static void log_debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    _log_print("DEBUG", LOG_COLOR_CYAN, format, args);
    va_end(args);
}

static void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    _log_print("ERROR", LOG_COLOR_RED, format, args);
    va_end(args);
}
