// File: src/error_handler.h
#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <errno.h>   // Added for standard error codes
#include <fcntl.h>   // Added for file locking macros (LOCK_EX, LOCK_NB, LOCK_UN)

// Define ETXTBSY if not available
#ifndef ETXTBSY
#define ETXTBSY 26
#endif

// Define file locking macros if not available
#ifndef LOCK_EX
#define LOCK_EX 2
#endif

#ifndef LOCK_NB
#define LOCK_NB 4
#endif

#ifndef LOCK_UN
#define LOCK_UN 8
#endif

// Enum for error types
typedef enum {
    MEMORY_ERROR,
    FILE_ACCESS_ERROR,
    INVALID_ARGUMENT,
    BAD_FILE_DESCRIPTOR,
    WRONG_DEVICE_COMMAND,
    DEVICE_ERROR,
    NULL_ERROR,       // Added NULL_ERROR for null pointer dereference
    UNKNOWN_ERROR,
    TXT_BUSY,         // Added TXT_BUSY for text file busy error
    DEVICE_ERROR_ACCESS_FAILURE,
    DEVICE_BUSY       // Added DEVICE_BUSY for device busy state
} ErrorType;

// Function to handle errors
void handle_error(ErrorType type, const char *message, int error_code);

#endif // ERROR_HANDLER_H
