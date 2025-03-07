#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096
#define NUM_PAGES 20
#define PATTERN_SIZE 128
#define NUM_ITERATIONS 3

void sequential_test(void) {
    printf(1, "\n=== Sequential Access Test ===\n");
    char *mem = sbrk(PGSIZE * NUM_PAGES);
    if(mem == (char*)-1) {
        printf(1, "sbrk failed\n");
        exit();
    }

    // Write pattern sequentially
    printf(1, "Writing pattern sequentially across %d pages...\n", NUM_PAGES);
    for(int i = 0; i < NUM_PAGES; i++) {
        printf(1, "Page %d (0x%x): ", i, (uint)(mem + i * PGSIZE));
        for(int j = 0; j < PATTERN_SIZE; j++) {
            mem[i * PGSIZE + j] = 'A' + (i % 26);
        }
        printf(1, "Wrote pattern '%c'\n", 'A' + (i % 26));
    }

    // Verify pattern
    printf(1, "\nVerifying pattern...\n");
    for(int i = 0; i < NUM_PAGES; i++) {
        char expected = 'A' + (i % 26);
        if(mem[i * PGSIZE] != expected) {
            printf(1, "Verification failed at page %d!\n", i);
            exit();
        }
    }
    printf(1, "Sequential test passed!\n");
}

void random_access_test(void) {
    printf(1, "\n=== Random Access Test ===\n");
    char *mem = sbrk(PGSIZE * NUM_PAGES);
    if(mem == (char*)-1) {
        printf(1, "sbrk failed\n");
        exit();
    }

    // Access pages in a non-sequential pattern
    int pattern[] = {5, 0, 15, 3, 10, 7, 12, 1, 8, 14};
    int pattern_len = 10;

    printf(1, "Accessing pages in random order...\n");
    for(int i = 0; i < pattern_len; i++) {
        int page = pattern[i];
        if(page < NUM_PAGES) {
            printf(1, "Accessing page %d (0x%x)\n", page, (uint)(mem + page * PGSIZE));
            mem[page * PGSIZE] = 'X';
        }
    }
}

void sparse_access_test(void) {
    printf(1, "\n=== Sparse Access Test ===\n");
    char *mem = sbrk(PGSIZE * NUM_PAGES);
    if(mem == (char*)-1) {
        printf(1, "sbrk failed\n");
        exit();
    }

    // Access every third page
    printf(1, "Accessing every third page...\n");
    for(int i = 0; i < NUM_PAGES; i += 3) {
        printf(1, "Page %d (0x%x): Writing\n", i, (uint)(mem + i * PGSIZE));
        mem[i * PGSIZE] = 'S';
    }
}

void realloc_test(void) {
    printf(1, "\n=== Reallocation Test ===\n");
    
    for(int iter = 0; iter < NUM_ITERATIONS; iter++) {
        printf(1, "\nIteration %d:\n", iter + 1);
        
        // Allocate
        printf(1, "Allocating %d pages...\n", NUM_PAGES/2);
        char *mem = sbrk(PGSIZE * (NUM_PAGES/2));
        if(mem == (char*)-1) {
            printf(1, "sbrk failed\n");
            exit();
        }

        // Use memory
        for(int i = 0; i < NUM_PAGES/2; i++) {
            mem[i * PGSIZE] = 'R';
        }

        // Deallocate
        printf(1, "Deallocating...\n");
        sbrk(-PGSIZE * (NUM_PAGES/2));
    }
}

int
main(int argc, char *argv[])
{
    printf(1, "Starting Memory Allocator Stress Test\n");
    printf(1, "Mode: %s\n", 
        #ifdef LOCALITY_ALLOCATOR
            "LOCALITY (allocates 3 pages at once)"
        #else
            "LAZY (allocates 1 page at a time)"
        #endif
    );
    printf(1, "Page size: %d bytes\n", PGSIZE);
    printf(1, "Testing %d pages\n", NUM_PAGES);

    sequential_test();
    random_access_test();
    sparse_access_test();
    realloc_test();

    printf(1, "\nAll tests completed successfully!\n");
    exit();
}