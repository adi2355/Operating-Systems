#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

void
test_alloc(void)
{
  printf(1, "Starting sbrk allocation test...\n");

  // Test 1: Single page allocation
  printf(1, "Test 1: Single page allocation\n");
  char *mem1 = sbrk(PGSIZE);
  if(mem1 == (char*)-1) {
    printf(1, "sbrk failed\n");
    exit();
  }
  printf(1, "Allocated page at 0x%x\n", mem1);
  
  // Write to memory to trigger page fault
  *mem1 = 'A';
  printf(1, "Successfully wrote to memory: %c\n", *mem1);

  // Test 2: Multiple page allocation
  printf(1, "\nTest 2: Multiple page allocation\n");
  char *mem2 = sbrk(PGSIZE * 2);
  if(mem2 == (char*)-1) {
    printf(1, "sbrk failed\n");
    exit();
  }
  printf(1, "Allocated 2 pages at 0x%x\n", mem2);
  
  // Write to both pages
  mem2[0] = 'B';
  mem2[PGSIZE] = 'C';
  printf(1, "Successfully wrote to first page: %c\n", mem2[0]);
  printf(1, "Successfully wrote to second page: %c\n", mem2[PGSIZE]);

  // Test 3: Negative allocation (shrink)
  printf(1, "\nTest 3: Negative allocation\n");
  if(sbrk(-PGSIZE * 3) == (char*)-1) {
    printf(1, "sbrk negative allocation failed\n");
    exit();
  }
  printf(1, "Successfully shrunk heap by 3 pages\n");

  printf(1, "\nAll tests completed successfully!\n");
}

int
main(void)
{
  test_alloc();
  exit();
}