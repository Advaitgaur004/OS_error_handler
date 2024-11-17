#include "recovery.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

// Maximum retry attempts and delay in seconds for error recovery
#define MAX_RETRIES 3
#define RETRY_DELAY 2 // seconds
#define TEMP_FILE "/temp/recovery_file.txt" // Temporary file for fallback

// Mock resource pointers simulating allocated memory for testing memory errors
void *allocated_memory_1 = NULL;
void *allocated_memory_2 = NULL;

// Function to recover from memory allocation error
void recover_from_memory_error() {
    printf("Attempting to recover from MEMORY_ERROR...\n");

    // Simulate memory availability check
    if (malloc(1) == NULL) {  
        fprintf(stderr, "Insufficient memory. Attempting cleanup...\n");

        // Cleanup by freeing previously allocated memory
        if (allocated_memory_1) {
            free(allocated_memory_1);
            printf("Freed allocated_memory_1.\n");
            allocated_memory_1 = NULL;
        }
        if (allocated_memory_2) {
            free(allocated_memory_2);
            printf("Freed allocated_memory_2.\n");
            allocated_memory_2 = NULL;
        }

        // Retry memory allocation after cleanup
        if (malloc(1) == NULL){
            fprintf(stderr, "Memory allocation failed after recovery attempt. Exiting...\n");
            exit(EXIT_FAILURE);  // Exit if memory allocation fails after cleanup
        } 
    }
    else {
        printf("Memory allocation successful after recovery attempt.\n");
    }
}

// Function to recover from file access errors
void recover_from_file_access_error(const char *filepath) {
    // Retry file access up to MAX_RETRIES
    for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        printf("Retrying file access (Attempt %d)...\n", attempt);
        FILE *file = fopen(filepath, "r");
        if (file != NULL) {
            printf("File accessed successfully on attempt %d.\n", attempt);
            fclose(file);
            return; // Return if file is accessed successfully
        }
        // Print error message on failure
        fprintf(stderr, "Attempt %d failed: %s\n", attempt, strerror(errno));
        sleep(RETRY_DELAY); // Delay before retry
    }

    // Fallback: If all attempts to access the file fail, create a temporary file
    printf("Failed to recover from FILE_ACCESS_ERROR after %d attempts.\n", MAX_RETRIES);
    printf("Attempting to create a temporary file as fallback...\n");
    FILE *temp_file = fopen(TEMP_FILE, "w");
    if (temp_file != NULL) {
        fprintf(temp_file, "Temporary file created as fallback.\n");
        fclose(temp_file);
        printf("Temporary file created at %s.\n", TEMP_FILE);
    } else {
        // Print error message if temporary file creation fails
        fprintf(stderr, "Failed to create temporary file: %s\n", strerror(errno));
    }
}

// Function to recover from device errors
void recover_from_device_error(const char *device_path) {
    printf("Attempting to recover from DEVICE_ERROR...\n");

    // Retry device reinitialization up to MAX_RETRIES
    for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        printf("Reinitializing device %s (Attempt %d)...\n", device_path, attempt);
        int fd = open(device_path, O_RDWR);  // Try opening the device
        if (fd != -1) {
            printf("Device reinitialized successfully on attempt %d.\n", attempt);
            close(fd);  // Close device file descriptor
            return; // Return if device reinitialized successfully
        }
        // Print error message on failure
        fprintf(stderr, "Attempt %d failed: %s\n", attempt, strerror(errno));
        sleep(RETRY_DELAY); // Delay before retry
    }

    // Fallback: If all attempts to reinitialize the device fail, switch to a backup device
    printf("Failed to recover from DEVICE_ERROR after %d attempts. Switching to backup.\n", MAX_RETRIES);
    const char *backup_device_path = "/dev/backup_device";
    int fd = open(backup_device_path, O_RDWR);  // Try opening backup device
    if (fd != -1) {
        printf("Backup device %s initialized successfully.\n", backup_device_path);
        close(fd);  // Close backup device file descriptor
    } else {
        // Print error message if backup device initialization fails
        fprintf(stderr, "Failed to initialize backup device: %s\n", strerror(errno));
    }
}

// Function to recover from NULL pointer errors
void recover_from_null_error() {
    printf("Attempting to recover from NULL_ERROR...\n");

    // Assign default values by allocating memory to a new pointer
    void *recovered_pointer = malloc(1024);  // Allocate 1KB memory
    if (recovered_pointer != NULL) {
        printf("NULL pointer recovery successful. Memory allocated.\n");
        free(recovered_pointer);  // Free the allocated memory
    } else {
        // Print error message if recovery fails due to memory allocation failure
        fprintf(stderr, "Failed to recover from NULL_ERROR due to memory allocation failure.\n");
        exit(EXIT_FAILURE);  // Exit if NULL pointer recovery fails
    }
}

// Main recovery function to handle different error types
void recover_from_error(ErrorType type) {
    switch (type) {
        case MEMORY_ERROR:
            recover_from_memory_error();  // Recover from memory errors
            break;
        case FILE_ACCESS_ERROR:
            recover_from_file_access_error("file.txt");  // Recover from file access errors
            break;
        case DEVICE_ERROR:
            recover_from_device_error("/dev/device");  // Recover from device errors
            break;
        case NULL_ERROR:
            recover_from_null_error();  // Recover from NULL pointer errors
            break;
        default:
            // If an unknown error type is encountered, exit the program
            printf("Unknown error type. Unable to recover.\n");
            exit(EXIT_FAILURE);
    }
}
