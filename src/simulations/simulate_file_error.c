// File: src/simulations/simulate_file_error.c
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "error_handler.h"
#include <stdlib.h>
#include <sys/ioctl.h>

#define MY_IOCTL_CMD 0x1234

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <error_code (1: FILE_ACCESS_ERROR, 2: INVALID_ARGUEMENT, 3: TXT_BUSY , 4:BAD_FILE_NUMBER)>\n", argv[0]);
        return -1;
    }
    int input_error = atoi(argv[1]);
    

    switch (input_error) {
        case 1:
            printf("Simulating file access error...\n");
            FILE *file = fopen("build/supper.txt", "r");
            if (file == NULL && errno==ENOENT) {
                printf("File Access Error: %s\n", strerror(errno));
                perror("");
                handle_error(FILE_ACCESS_ERROR, strerror(errno), errno);
                return errno;
            }
            fclose(file);
            break;
        case 2:
            printf("Simulating file access error...\n");
            FILE *file1 = fopen("build/supper.txt", "s");
            if (file1 == NULL && errno==EINVAL) {
                printf("File Access Error: %s\n", strerror(errno));
                perror("");
                handle_error(INVALID_ARGUMENT, strerror(errno), errno);
                return errno;
            }
            fclose(file);
            break;
            break;
        case 3:
            printf("Simulating file access error TXT_BUSY\n");
            int fd = open("build/sleep", O_WRONLY|O_TRUNC);
            
            if (fd == -1) {
                printf("Device Error: %s\n", strerror(errno));
                
                if (errno == ETXTBSY) {
                    handle_error(TXT_BUSY, strerror(errno), errno);
                }
                return errno;
            }
            else{
                printf("run ./sleep in another terminal to get this error as file not running\n");
            }
            close(fd);
            break;
        case 4:
            printf("Simulating file access error BAD_FILE_DESCRIPTOR\n");
            fd = open("/build/sleep", O_RDONLY);
    
            // Attempt to issue an IOCTL command to the device
            int ret = ioctl(fd, MY_IOCTL_CMD, NULL);

            if (ret == -1 && errno == EBADF) {
                handle_error(BAD_FILE_DESCRIPTOR, strerror(errno), errno);
            }

            close(fd);
            break;
        default:
            fprintf(stderr, "Invalid error code. Use 1, 2, 3 or 4.\n");
            return -1;
    }
    
    return 0;
}