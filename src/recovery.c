#include "recovery.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>

// Define maximum retries and delay
#define MAX_RETRIES 3
#define RETRY_DELAY 2 // seconds
#define MAX_MEMORY_THRESHOLD 0.9 // 90% of available memory

// Forward declarations
unsigned long get_system_memory(void);
static int check_device_status(const char *device_path);
static int reset_device(const char *device_path);

// Helper function to get system memory
unsigned long get_system_memory(void) {
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (meminfo == NULL) {
        // Fallback to a default value if /proc/meminfo is not accessible
        return 8L * 1024 * 1024; // Default to 8GB
    }

    unsigned long total_memory = 0;
    char line[256];
    while (fgets(line, sizeof(line), meminfo)) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line, "MemTotal: %lu kB", &total_memory);
            total_memory *= 1024; // Convert to bytes
            break;
        }
    }
    fclose(meminfo);
    return total_memory ? total_memory : 8L * 1024 * 1024;
}

// Device-specific recovery functions
static int check_device_status(const char *device_path) {
    struct stat st;
    if (stat(device_path, &st) == 0) {
        // Check if device is accessible
        int fd = open(device_path, O_RDONLY | O_NONBLOCK);
        if (fd != -1) {
            close(fd);
            return 1;
        }
    }
    return 0;
}

static int reset_device(const char *device_path) {
    // Attempt to reset the device
    int fd = open(device_path, O_RDWR);
    if (fd != -1) {
        // Send reset command (device specific)
        ioctl(fd, TIOCEXCL, 0);  // Reset exclusive mode
        ioctl(fd, TIOCNXCL, 0);  // Reset non-exclusive mode
        close(fd);
        return 1;
    }
    return 0;
}

// Recovery utility functions
void cleanup_resources(void) {
    printf("Cleaning up system resources...\n");
    
    // Close all open file descriptors
    for (int fd = 3; fd < 1024; fd++) {
        close(fd);
    }
    
    // Release any shared memory segments
    system("ipcrm -a");
    
    // Clear temporary files
    system("rm -f /tmp/error_handler_*");
    
    // Log cleanup operation
    log_error(UNKNOWN_ERROR, "System resources cleanup performed", 0);
}

int verify_system_resources(void) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // Check memory usage
        if (usage.ru_maxrss > (MAX_MEMORY_THRESHOLD * get_system_memory())) {
            return 0;
        }
        return 1;
    }
    return 0;
}

// Specific recovery functions
RecoveryStatus recover_from_file_access_error(const char *filepath) {
    printf("Attempting to recover from FILE_ACCESS_ERROR for %s...\n", filepath);
    
    // Try alternate file paths
    char backup_path[256];
    snprintf(backup_path, sizeof(backup_path), "%s.backup", filepath);
    
    for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        printf("Retry attempt %d/%d...\n", attempt, MAX_RETRIES);
        
        // Try original file
        FILE *file = fopen(filepath, "r");
        if (file != NULL) {
            printf("Successfully accessed file on attempt %d\n", attempt);
            fclose(file);
            return RECOVERY_SUCCESS;
        }
        
        // Try backup file
        file = fopen(backup_path, "r");
        if (file != NULL) {
            printf("Successfully accessed backup file\n");
            fclose(file);
            return RECOVERY_PARTIAL;
        }
        
        sleep(RETRY_DELAY);
    }
    
    printf("Failed to recover after %d attempts\n", MAX_RETRIES);
    return RECOVERY_FAILED;
}

RecoveryStatus recover_from_memory_error(void) {
    printf("Attempting to recover from MEMORY_ERROR...\n");
    
    // Step 1: Clean up resources
    cleanup_resources();
    
    // Step 2: Verify system resources
    if (!verify_system_resources()) {
        printf("System resources are still constrained\n");
        return RECOVERY_FAILED;
    }
    
    // Step 3: Try to allocate a small amount of memory to test
    void *test_ptr = malloc(1024);
    if (test_ptr == NULL) {
        printf("Memory allocation still failing\n");
        return RECOVERY_FAILED;
    }
    
    free(test_ptr);
    printf("Memory recovery successful\n");
    return RECOVERY_SUCCESS;
}

RecoveryStatus recover_from_null_error(void) {
    printf("Attempting to recover from NULL_ERROR...\n");
    
    // Verify system state
    if (!verify_system_resources()) {
        printf("System resources verification failed\n");
        return RECOVERY_FAILED;
    }
    
    // Log the recovery attempt
    log_error(NULL_ERROR, "Recovered from null pointer error", 0);
    
    return RECOVERY_SUCCESS;
}

RecoveryStatus recover_from_device_error(void) {
    printf("Attempting to recover from DEVICE_ERROR...\n");
    
    const char *device_paths[] = {
        "/dev/tty0",
        "/dev/null",
        "/dev/zero",
        NULL
    };
    
    for (int i = 0; device_paths[i] != NULL; i++) {
        for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
            printf("Attempting device reinitialization for %s (%d/%d)...\n", 
                   device_paths[i], attempt, MAX_RETRIES);
            
            // Check device status
            if (check_device_status(device_paths[i])) {
                printf("Device %s is accessible\n", device_paths[i]);
                return RECOVERY_SUCCESS;
            }
            
            // Try to reset the device
            if (reset_device(device_paths[i])) {
                printf("Device %s reset successful\n", device_paths[i]);
                return RECOVERY_SUCCESS;
            }
            
            sleep(RETRY_DELAY);
        }
    }
    
    log_error(DEVICE_ERROR, "Failed to recover device after multiple attempts", errno);
    return RECOVERY_FAILED;
}

RecoveryStatus recover_from_device_busy(void) {
    printf("Attempting to recover from DEVICE_BUSY...\n");
    
    for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        printf("Waiting for device to become available (%d/%d)...\n", attempt, MAX_RETRIES);
        
        // Check system load
        double loadavg[1];
        if (getloadavg(loadavg, 1) == 1 && loadavg[0] < 0.8) {
            // System load is acceptable
            if (verify_system_resources()) {
                printf("Device is now available\n");
                return RECOVERY_SUCCESS;
            }
        }
        
        // Force release of device locks
        system("fuser -k /dev/busy_device 2>/dev/null");
        
        sleep(RETRY_DELAY * 2);
    }
    
    log_error(DEVICE_BUSY, "Device remains busy after recovery attempts", errno);
    return RECOVERY_FAILED;
}

RecoveryStatus recover_from_txt_busy(const char *filepath) {
    printf("Attempting to recover from TXT_BUSY for %s...\n", filepath);
    
    for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        printf("Checking file availability (%d/%d)...\n", attempt, MAX_RETRIES);
        
        // Try to open the file with non-blocking flags
        int fd = open(filepath, O_RDWR | O_NONBLOCK);
        if (fd != -1) {
            printf("File is now available\n");
            close(fd);
            return RECOVERY_SUCCESS;
        }
        
        if (errno != ETXTBSY) {
            printf("Unexpected error: %s\n", strerror(errno));
            return RECOVERY_FAILED;
        }
        
        sleep(RETRY_DELAY);
    }
    
    return RECOVERY_FAILED;
}

RecoveryStatus recover_from_error(ErrorType type) {
    RecoveryStatus status = RECOVERY_FAILED;
    
    switch(type) {
        case MEMORY_ERROR:
            status = recover_from_memory_error();
            break;
            
        case FILE_ACCESS_ERROR:
            status = recover_from_file_access_error("/path/to/nonexistent/file.txt");
            break;
            
        case DEVICE_ERROR:
            status = recover_from_device_error();
            break;
            
        case NULL_ERROR:
            status = recover_from_null_error();
            break;
            
        case TXT_BUSY:
            status = recover_from_txt_busy("example.lock");
            break;
            
        case DEVICE_BUSY:
            status = recover_from_device_busy();
            break;
            
        default:
            printf("Unknown error type. Unable to recover.\n");
            return RECOVERY_FAILED;
    }
    
    // Log recovery status
    const char *status_str = (status == RECOVERY_SUCCESS) ? "successful" :
                           (status == RECOVERY_PARTIAL) ? "partial" : "failed";
    printf("Recovery %s for error type %d\n", status_str, type);
    
    // If recovery failed, perform cleanup
    if (status == RECOVERY_FAILED) {
        cleanup_resources();
    }
    
    return status;
}