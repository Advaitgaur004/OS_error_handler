// File: src/recovery.h
#ifndef RECOVERY_H
#define RECOVERY_H

#include "error_handler.h"

// Recovery status enum
typedef enum {
    RECOVERY_SUCCESS,
    RECOVERY_PARTIAL,
    RECOVERY_FAILED
} RecoveryStatus;

// Main recovery function
RecoveryStatus recover_from_error(ErrorType type);

// Specific recovery functions
RecoveryStatus recover_from_file_access_error(const char *filepath);
RecoveryStatus recover_from_memory_error(void);
RecoveryStatus recover_from_null_error(void);
RecoveryStatus recover_from_device_error(void);
RecoveryStatus recover_from_device_busy(void);
RecoveryStatus recover_from_txt_busy(const char *filepath);

// Recovery utility functions
void cleanup_resources(void);
int verify_system_resources(void);

#endif // RECOVERY_H