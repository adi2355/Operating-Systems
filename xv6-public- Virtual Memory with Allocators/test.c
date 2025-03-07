#include "types.h"
#include "user.h"

void check_allocator(void) {
    printf(1, "Checking allocator type...\n");
    
    // Allocate memory to trigger page fault
    char *mem = sbrk(4096 * 4);  // Request 4 pages
    
    // Write to trigger page faults
    for(int i = 0; i < 4; i++) {
        mem[i * 4096] = 'A' + i;
        printf(1, "Wrote to page %d\n", i);
    }
}

int main(void) {
    check_allocator();
    exit();
}