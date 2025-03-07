#include "types.h"
#include "stat.h"
#include "user.h"

// Declaration of new syscalls
int set_lottery_tickets(int tickets);
int get_lottery_tickets(int pid);

// Function to simulate CPU-intensive work
void cpu_bound_workload() {
    int i;
    volatile int counter = 0;
    for (i = 0; i < 100000000; i++) {
        counter += i;
    }
    exit();
}

void advanced_scheduler_test() {
    int pid;
    int tickets;
    int i;  // Declare loop variable outside the loop (C89 compliant)

    // Fork process 1
    pid = fork();
    if (pid == 0) {
        // Child process 1: Set tickets to 10
        set_lottery_tickets(10);
        cpu_bound_workload();  // Run a CPU-bound task
    } else {
        // Fork process 2
        pid = fork();
        if (pid == 0) {
            // Child process 2: Set tickets to 50
            set_lottery_tickets(50);
            cpu_bound_workload();  // Run a CPU-bound task
        } else {
            // Fork process 3
            pid = fork();
            if (pid == 0) {
                // Child process 3: Set tickets to 90
                set_lottery_tickets(90);
                cpu_bound_workload();  // Run a CPU-bound task
            } else {
                // Parent process: Check tickets and wait for children
                int child_pids[3] = {pid - 2, pid - 1, pid};

                // Get and print the tickets for each process
                for (i = 0; i < 3; i++) {  // Loop variable declared outside
                    tickets = get_lottery_tickets(child_pids[i]);
                    printf(1, "Process %d has %d tickets\n", child_pids[i], tickets);
                }

                // Wait for all child processes to finish
                for (i = 0; i < 3; i++) {  // Loop variable declared outside
                    wait();
                }

                printf(1, "Lottery scheduler test finished\n");
            }
        }
    }
}

int main(void) {
    printf(1, "Starting advanced scheduler test\n");
    advanced_scheduler_test();
    exit();
}
