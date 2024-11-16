// File: src/logger.c
#include "logger.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#define LOG_FILE "logs/error_log.log"
#define MAX_LOG_SIZE 5242880 // 5MB

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to get current timestamp
const char* current_timestamp() {
    static char buffer[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, sizeof(buffer)-1, "%Y-%m-%d %H:%M:%S", t);
    return buffer;
}

// Function to convert ErrorType to string
const char* error_type_to_string(ErrorType type) {
    switch(type) {
        case MEMORY_ERROR:
            return "MEMORY_ERROR";
        case FILE_ACCESS_ERROR:
            return "FILE_ACCESS_ERROR";
        case DEVICE_ERROR:
            return "DEVICE_ERROR";
        case NULL_ERROR:
            return "NULL_ERROR";
        case TXT_BUSY:
            return "TXT_BUSY";
        case DEVICE_BUSY:
            return "DEVICE_BUSY";
        case DEVICE_ERROR_ACCESS_FAILURE:
            return "DEVICE_ERROR_ACCESS_FAILURE";
        case INVALID_ARGUMENT:
            return "INVALID_ARGUMENT";
        case BAD_FILE_DESCRIPTOR:
            return "BAD_FILE_DESCRIPTOR";
        case WRONG_DEVICE_COMMAND:
            return "WRONG_DEVICE_COMMAND";
        default:
            return "UNKNOWN_ERROR";
    }
}

// Function to rotate logs if needed
void rotate_logs_if_needed() {
    struct stat st;
    if (stat(LOG_FILE, &st) == 0) {
        if (st.st_size >= MAX_LOG_SIZE) {
            // Rename the current log file with a timestamp
            char new_name[256];
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            strftime(new_name, sizeof(new_name), "logs/error_log_%Y%m%d%H%M%S.log", t);
            rename(LOG_FILE, new_name);
        }
    }
}

void ensure_log_directory_exists() {
    struct stat st = {0};
    if (stat("logs", &st) == -1) {
        if (mkdir("logs", 0755) != 0 && errno != EEXIST) {
            fprintf(stderr, "Failed to create logs directory: %s\n", strerror(errno));
        }
    }
}

void log_error(ErrorType type, const char *message, int error_code) {
    pthread_mutex_lock(&log_mutex);
    ensure_log_directory_exists();
    rotate_logs_if_needed();
    FILE *file = fopen(LOG_FILE, "a");
    if (!file) {
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    fprintf(file, "[%s] %s: %s (Error Code: %d)\n", current_timestamp(), error_type_to_string(type), message, error_code);
    fclose(file);
    pthread_mutex_unlock(&log_mutex);
}