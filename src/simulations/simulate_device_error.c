// File: src/simulations/simulate_device_error.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "error_handler.h"
#include <stdlib.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#define MY_IOCTL_CMD 0x1234

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <error_code (1: DEVICE_NOT_FOUND, 2: ENOTTY, 3: DEVICE_ACCESS_FAILURE , 4:DEVICE_BUSY)>\n", argv[0]);
        return -1;
    }
    int input_error = atoi(argv[1]);
    int fd;

    switch (input_error) {
        case 1:
            printf("Simulating device access error DEVICE_NOT_FOUND\n");
            fd = open("/dev/nonexistent_device", O_RDONLY);
            
            if (fd == -1) {
                printf("Device Error: %s\n", strerror(errno));
                
                if (errno == ENOENT) {
                    handle_error(DEVICE_ERROR, strerror(errno), errno);
                }
                return errno;
            }
            close(fd);
            break;
        case 2:
            printf("Simulating device access error WRONG_DEVICE_COMMAND\n");
            int fd = open("build/sleep", O_RDONLY);
    
            // Attempt to issue an IOCTL command to the device
            int ret = ioctl(fd, MY_IOCTL_CMD, NULL);

            if (ret == -1 && errno == ENOTTY) {
                handle_error(WRONG_DEVICE_COMMAND, strerror(errno), errno);
            }


            close(fd);
            break;
        case 3:
            printf("Simulating device access error DEVICE_ACCESS_FAILURE\n");
            fd = open("build/access.txt", O_RDWR);
            
            if (fd == -1) {
                printf("Device Error: %s\n", strerror(errno));
                
                if (errno == EACCES) {
                    handle_error(DEVICE_ERROR_ACCESS_FAILURE, strerror(errno), errno);
                }
                else{
                    printf("different error generated");
                }
                return errno;
            }
            else{
                printf("unkown problem in the error creation");
            }
            close(fd);
            break;
        case 4:
            printf("Simulating device access error DEVICE_BUSY/RESOURCE_UNAVAILBLE\n");
            fd = open("build/example.lock", O_RDWR);
            
            if (fd == -1) {
                perror("open");
                return 1;
            }
            // Attempt to lock the file again, which will fail with EAGAIN
            if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
                if (errno == EBUSY || errno ==EAGAIN) {
                    handle_error(DEVICE_BUSY, strerror(errno), errno);
                }
            }else{printf("noerror\n");}
            
            // Clean up
            flock(fd, LOCK_UN);
            close(fd);
            break;
        default:
            fprintf(stderr, "Invalid error code. Use 1, 2, 3 or 4.\n");
            return -1;
    }
    
    return 0;
}