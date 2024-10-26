#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t process1 = fork();

    if (process1 < 0) {
        // Error occurred
        perror("Fork failed");
        exit(1);
    } else if (process1 == 0) {
        // Child process (Process 1)
        while (1) {
            printf("I am Process 1\n");
            sleep(1); // Slows down the display
        }
    } else {
        // Parent process (Process 2)
        while (1) {
            printf("I am Process 2\n");
            sleep(1); // Slows down the display
        }
    }

    return 0;
}
