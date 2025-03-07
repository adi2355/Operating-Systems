// Test that fork fails gracefully.
// Tiny executable so that the limit can be filling the proc table.

#include "types.h"
#include "stat.h"
#include "user.h"

#define N  1000

void
my_print_pid(int fd, int pid) {
    char buf[16]; // Buffer to hold the PID as a string
    int i = 0;

    // Handle special case of PID being zero
    if (pid == 0) {
        buf[i++] = '0';
    } else {
        // Convert PID to string
        while (pid > 0) {
            buf[i++] = (pid % 10) + '0'; // Get last digit and convert to char
            pid /= 10; // Remove last digit
        }
    }

    // Reverse the string to correct the order
    int j;
    for (j = 0; j < i / 2; j++) {
        char temp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = temp;
    }

    buf[i] = '\n'; // Null-terminate the string
    write(fd, buf, i + 1); // Write to the file descriptor
}

void
forktest(void) {
    int n, pid;

    write(1, "fork test\n", 10); // Use write directly

    for(n = 0; n < N; n++) {
        pid = fork();
        if (pid < 0)
            break;
        if (pid == 0) {
            my_print_pid(1, getpid()); // Print child PID
            sleep(100); // Keep the child process running for a while (100 ticks)
            exit(); // Exit after sleeping
        }
    }

    if (n == N) {
        write(1, "fork claimed to work N times!\n", 31); // Use write directly
        exit();
    }

    for (; n > 0; n--) {
        if (wait() < 0) {
            write(1, "wait stopped early\n", 20); // Use write directly
            exit();
        }
    }

    if (wait() != -1) {
        write(1, "wait got too many\n", 19); // Use write directly
        exit();
    }

    write(1, "fork test OK\n", 13); // Use write directly
}

int
main(void) {
    forktest();
    exit();
}
