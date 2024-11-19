#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>

// Define LOCK macros if not available
#ifndef LOCK_EX
#define LOCK_EX 2
#endif

#ifndef LOCK_NB
#define LOCK_NB 4
#endif

#ifndef LOCK_UN
#define LOCK_UN 8
#endif

int main(void) {

    int fd = open("build/example.lock", O_CREAT | O_RDWR);

    if (fd == -1) {
        perror("");
        printf("%d",errno);
        return 1;
    }

    // Lock the file exclusively and non-blocking
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        perror("flock");
        close(fd);
        return 1;
    }
    int a;
    printf("File locked. Do not give input until work is done...\n");
    scanf("%d",&a); //changed to scanf as to give more flexible time and unlock is possible
    //sleep(10000); // Simulate a long-running process

    // Unlock the file
    if (flock(fd, LOCK_UN) == -1) {
        perror("flock unlock");
    }
    close(fd);
    return 0;
}
