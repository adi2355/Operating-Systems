#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define NUM_TESTS 3
#define NUM_ITERATIONS 3  // Reduced iterations to avoid system stress
#define STRESSFS_DURATION 20  // Reduced duration
#define CPU_WORKLOAD 5000  // Increased CPU workload

struct test_result {
    int pid;
    int start_time;
    int end_time;
    int runtime;
    char name[32];
};

// Modified CPU-bound task with heavier workload
void cpu_bound_task(int iterations) {
    int i, j, k;
    volatile int sum = 0;  // Use volatile to prevent optimization
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < 1000; j++) {
            for (k = 0; k < 100; k++) {
                sum += k;  // Actually do some computation
            }
        }
    }
}

// Modified stressfs to be less intensive
void run_stressfs(int duration) {
    int pid = fork();
    if (pid == 0) {
        // Create smaller files
        int fd = open("test.txt", O_CREATE | O_RDWR);
        if (fd >= 0) {
            char buf[512];
            for (int i = 0; i < 10; i++) {  // Reduced number of writes
                write(fd, buf, sizeof(buf));
            }
            close(fd);
            unlink("test.txt");
        }
        exit();
    }
    wait();
}

// Function to run ls and measure its execution time
int run_ls() {
    int start = uptime();
    int pid = fork();
    if (pid == 0) {
        char *args[] = {"ls", 0};
        exec("ls", args);
        exit();
    }
    wait();
    return uptime() - start;
}

// Modified pipe test to be less intensive
int test_pipe() {
    int start = uptime();
    int pipefd[2];
    char buf[512];
    
    if (pipe(pipefd) < 0) {
        printf(1, "pipe creation failed\n");
        return -1;
    }

    int pid = fork();
    if (pid == 0) {  // child writes
        close(pipefd[0]);
        for (int i = 0; i < 20; i++) {  // Reduced iterations
            write(pipefd[1], buf, sizeof(buf));
        }
        close(pipefd[1]);
        exit();
    } else {  // parent reads
        close(pipefd[1]);
        while (read(pipefd[0], buf, sizeof(buf)) > 0);
        close(pipefd[0]);
        wait();
    }
    
    return uptime() - start;
}

int main(void) {
    printf(1, "\n=== Scheduler Performance Test ===\n\n");
    
    struct test_result results[NUM_TESTS];
    int total_start = uptime();
    
    // Test 1: CPU-bound processes with proper timing
    printf(1, "Test 1: CPU-bound processes\n");
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        results[0].start_time = uptime();
        if ((results[0].pid = fork()) == 0) {
            strcpy(results[0].name, "CPU-bound");
            cpu_bound_task(CPU_WORKLOAD);
            exit();
        }
        wait();
        results[0].end_time = uptime();
        printf(1, "CPU Test %d completed in %d ticks\n", 
               i, results[0].end_time - results[0].start_time);
        sleep(10);  // Add small delay between tests
    }
    
    printf(1, "\nTest 2: I/O operations\n");
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int start = uptime();
        run_stressfs(STRESSFS_DURATION);
        int duration = uptime() - start;
        printf(1, "I/O Test %d completed in %d ticks\n", i, duration);
        sleep(10);  // Add small delay between tests
    }
    
    printf(1, "\nTest 3: Pipe operations\n");
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int pipe_time = test_pipe();
        printf(1, "Pipe Test %d completed in %d ticks\n", i, pipe_time);
        sleep(10);  // Add small delay between tests
    }
    
    printf(1, "\nReal-world test: ls command\n");
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int ls_time = run_ls();
        printf(1, "ls Test %d completed in %d ticks\n", i, ls_time);
        sleep(10);  // Add small delay between tests
    }
    
    int total_time = uptime() - total_start;
    printf(1, "\nTotal test suite execution time: %d ticks\n", total_time);
    
    #ifdef SJF
    printf(1, "\nSJF Predicted Job Lengths:\n");
    for (int i = 0; i < NUM_TESTS; i++) {
        if (results[i].pid > 0) {
            int predicted = sjf_job_length(results[i].pid);
            printf(1, "Process %d predicted length: %d\n", 
                   results[i].pid, predicted);
        }
    }
    #endif
    
    printf(1, "\n=== Test Complete ===\n");
    exit();
}