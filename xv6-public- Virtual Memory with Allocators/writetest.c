#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define BLOCKSIZE 512
#define KB (1024)
#define MB (1024*1024)

void
writeblock(int fd, char *data, int size) {
  if (write(fd, data, size) != size) {
    printf(1, "Write failed\n");
    exit();
  }
}

void
test_size(int nblocks, int expect_success) {
  int fd;
  char buf[BLOCKSIZE];
  int i;
  uint total_size = 0;
  uint progress = 0;

  printf(1, "\nTesting with %d blocks...\n", nblocks);
  printf(1, "Total size will be: %d bytes (%d MB)\n", 
         nblocks * BLOCKSIZE, (nblocks * BLOCKSIZE) / MB);
  printf(1, "Expected result: %s\n", expect_success ? "Success" : "Failure");
  
  // Create test file
  printf(1, "Creating file...\n");
  fd = open("largefile", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "Failed to create file\n");
    exit();
  }

  // Fill buffer with pattern
  for(i = 0; i < BLOCKSIZE; i++)
    buf[i] = i % 128;

  // Write blocks
  printf(1, "Writing blocks...\n");
  for(i = 0; i < nblocks; i++) {
    int result = write(fd, buf, BLOCKSIZE);
    if(result != BLOCKSIZE) {
      if(!expect_success) {
        printf(1, "Write failed as expected at block %d\n", i);
        close(fd);
        unlink("largefile");
        return;
      } else {
        printf(1, "ERROR: Write failed unexpectedly at block %d\n", i);
        exit();
      }
    }
    total_size += BLOCKSIZE;
    
    if (total_size * 100 / (nblocks * BLOCKSIZE) > progress) {
      progress = total_size * 100 / (nblocks * BLOCKSIZE);
      if(progress % 10 == 0)
        printf(1, "Progress: %d%% (%d MB written)\n", progress, total_size / MB);
    }
  }

  // Verify size
  struct stat st;
  if(fstat(fd, &st) < 0) {
    printf(1, "Failed to get file stats\n");
    exit();
  }
  printf(1, "Final file size: %d bytes (%d MB)\n", st.size, st.size / MB);
  
  // Test reading back
  printf(1, "Verifying file contents...\n");
  int orig_size = st.size;
  char readbuf[BLOCKSIZE];
  
  // Seek to beginning
  close(fd);
  fd = open("largefile", O_RDONLY);
  
  // Read and verify
  int total_read = 0;
  while(total_read < orig_size) {
    int n = read(fd, readbuf, BLOCKSIZE);
    if(n <= 0) {
      printf(1, "Read verification failed at offset %d\n", total_read);
      exit();
    }
    total_read += n;
  }
  printf(1, "Read verification successful\n");

  close(fd);
  printf(1, "Deleting file to test block freeing...\n");
  if(unlink("largefile") < 0) {
    printf(1, "Failed to delete file\n");
    exit();
  }
}

int
main(void)
{
  printf(1, "Starting large file tests...\n");
  
  // Test original limit
  test_size(140, 1);  // Should succeed
  test_size(141, 0);  // Should fail in original mode
  
  // Test with one double-indirect block
  test_size(16523, 1);  // Should succeed
  test_size(16524, 0);  // Should fail
  
  // Test with two double-indirect blocks
  test_size(32906, 1);  // Should succeed
  test_size(32907, 0);  // Should fail
  
  printf(1, "\nAll tests completed successfully!\n");
  exit();
}