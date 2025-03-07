#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

// Test function declarations
void test_calling_process(void);
void test_other_process(void);
void test_unscheduled_process(void);
void test_nonexistent_process(void);
void run_stress_test(void);

int
main(int argc, char *argv[])
{
  printf(1, "\n=== Testing ticks_running() implementation ===\n\n");

  // Test 1: Basic syscall functionality
  printf(1, "Test 1: Checking basic syscall functionality...\n");
  int pid = getpid();
  int ticks = ticks_running(pid);
  if(ticks >= 0) {
    printf(1, "PASS: syscall returns valid value (%d) for current process\n", ticks);
  } else {
    printf(1, "FAIL: syscall returned invalid value (%d)\n", ticks);
  }
  printf(1, "\n");

  // Test 2: Current process tracking
  printf(1, "Test 2: Testing current process time tracking...\n");
  test_calling_process();
  printf(1, "\n");

  // Test 3: Other process tracking
  printf(1, "Test 3: Testing other process time tracking...\n");
  test_other_process();
  printf(1, "\n");

  // Test 4: Unscheduled process
  printf(1, "Test 4: Testing unscheduled process...\n");
  test_unscheduled_process();
  printf(1, "\n");

  // Test 5: Nonexistent process
  printf(1, "Test 5: Testing nonexistent process...\n");
  test_nonexistent_process();
  printf(1, "\n");

  // Finally, run stress test
  printf(1, "Running stress test with ticks monitoring...\n");
  run_stress_test();

  exit();
}

void test_calling_process(void) {
  int pid = getpid();
  int ticks_start = ticks_running(pid);
  
  // Do some work
  int i;
  for(i = 0; i < 1000000; i++) { }
  
  int ticks_end = ticks_running(pid);
  
  if(ticks_end > ticks_start) {
    printf(1, "PASS: Process time increased (start: %d, end: %d)\n", 
           ticks_start, ticks_end);
  } else {
    printf(1, "FAIL: Process time did not increase (start: %d, end: %d)\n", 
           ticks_start, ticks_end);
  }
}

void test_other_process(void) {
  int parent_pid = getpid();
  int child_pid = fork();
  
  if(child_pid == 0) {  // Child
    // Do some work
    int i;
    for(i = 0; i < 1000000; i++) { }
    exit();
  } else if(child_pid > 0) {  // Parent
    int child_ticks = ticks_running(child_pid);
    printf(1, "Child process (PID: %d) ticks: %d\n", child_pid, child_ticks);
    wait();
  }
}

void test_unscheduled_process(void) {
  int child_pid = fork();
  
  if(child_pid == 0) {  // Child
    int ticks = ticks_running(getpid());
    if(ticks == 0) {
      printf(1, "PASS: New process has 0 ticks\n");
    } else {
      printf(1, "FAIL: New process has %d ticks (expected 0)\n", ticks);
    }
    exit();
  } else if(child_pid > 0) {  // Parent
    wait();
  }
}

void test_nonexistent_process(void) {
  int nonexistent_pid = 32767;  // Large number unlikely to be a valid PID
  int ticks = ticks_running(nonexistent_pid);
  
  if(ticks == -1) {
    printf(1, "PASS: Nonexistent process (PID: %d) returns -1\n", nonexistent_pid);
  } else {
    printf(1, "FAIL: Nonexistent process returned %d (expected -1)\n", ticks);
  }
}

void run_stress_test(void) {
  int fd, i;
  char path[] = "stressfs0";
  char data[256];
  int parent_pid = getpid();
  
  printf(1, "Stress test starting (Parent PID: %d)\n", parent_pid);
  
  memset(data, 'a', sizeof(data));

  // Create processes
  for(i = 0; i < 2; i++) {
    int pid = fork();
    if(pid < 0) {
      printf(1, "fork failed\n");
      exit();
    }
    if(pid == 0) {
      printf(1, "Child process created (PID: %d)\n", getpid());
      break;
    }
    sleep(10);
  }

  int current_pid = getpid();
  path[8] = '0' + i;

  // Monitor initial ticks
  int ticks_start = ticks_running(current_pid);
  printf(1, "PID %d: Initial ticks: %d\n", current_pid, ticks_start);

  // Perform I/O operations
  printf(1, "PID %d: Starting I/O operations\n", current_pid);
  fd = open(path, O_CREATE | O_RDWR);
  if(fd >= 0) {
    for(i = 0; i < 5; i++) {
      write(fd, data, sizeof(data));
    }
    close(fd);
  }

  fd = open(path, O_RDONLY);
  if(fd >= 0) {
    for(i = 0; i < 5; i++) {
      read(fd, data, sizeof(data));
    }
    close(fd);
  }

  // Get final ticks
  int ticks_end = ticks_running(current_pid);
  printf(1, "PID %d: Final ticks: %d (Total: %d)\n", 
         current_pid, ticks_end, ticks_end - ticks_start);

  unlink(path);

  if(current_pid == parent_pid) {
    // Parent waits for children
    int child_pid;
    for(i = 0; i < 2; i++) {
      child_pid = wait();
      printf(1, "Child process %d completed\n", child_pid);
    }
    printf(1, "Stress test complete\n");
  }
}