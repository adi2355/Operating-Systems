#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

void print_mem_access(char *mem, int pages) {
    int i;
    printf(1, "\nWriting to memory:\n");
    for(i = 0; i < pages; i++) {
        printf(1, "Accessing page %d at 0x%x\n", i, (uint)(mem + i * PGSIZE));
        mem[i * PGSIZE] = 'A' + i;
        printf(1, "Successfully wrote '%c'\n", mem[i * PGSIZE]);
        
        // Read back to verify
        printf(1, "Read back: '%c'\n\n", mem[i * PGSIZE]);
    }
}

int main(void) {
    printf(1, "Starting allocator test...\n");
    printf(1, "Mode: %s\n\n", 
        #ifdef LOCALITY_ALLOCATOR
            "LOCALITY"
        #else
            "LAZY"
        #endif
    );

    // Allocate 6 pages
    char *mem = sbrk(PGSIZE * 6);
    if(mem == (char*)-1) {
        printf(1, "sbrk failed\n");
        exit();
    }

    print_mem_access(mem, 6);

    printf(1, "Test completed successfully!\n");
    exit();
}