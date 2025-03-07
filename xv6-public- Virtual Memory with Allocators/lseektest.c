#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char buf[512];

int
main(void)
{
  int fd;

  // Create a test file
  fd = open("lseek.txt", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "test: cannot create lseek.txt\n");
    exit();
  }

  // Write initial content
  printf(1, "Writing 'Hello' to file...\n");
  if(write(fd, "Hello", 5) != 5){
    printf(1, "test: write error\n");
    close(fd);
    exit();
  }

  // Move file pointer 5 positions forward (creating a hole)
  printf(1, "Seeking 5 positions forward...\n");
  if(lseek(fd, 5) < 0){
    printf(1, "test: lseek error\n");
    close(fd);
    exit();
  }

  // Write more content after the hole
  printf(1, "Writing 'World' after the hole...\n");
  if(write(fd, "World", 5) != 5){
    printf(1, "test: write error\n");
    close(fd);
    exit();
  }

  // Close and reopen for reading
  close(fd);
  
  fd = open("lseek.txt", O_RDONLY);
  if(fd < 0){
    printf(1, "test: cannot open lseek.txt for reading\n");
    exit();
  }

  // Read and verify content
  int n = read(fd, buf, sizeof(buf));
  if(n < 0){
    printf(1, "test: read error\n");
    close(fd);
    exit();
  }

  printf(1, "File content (first %d bytes): ", n);
  for(int i = 0; i < n; i++) {
    if(buf[i] == 0)
      printf(1, "\\0");
    else
      printf(1, "%c", buf[i]);
  }
  printf(1, "\n");

  close(fd);
  printf(1, "Test completed successfully!\n");
  exit();
}