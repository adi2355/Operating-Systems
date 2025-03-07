#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(void)
{
  int fd;
  char buf[64];

  printf(1, "Test 1: Basic symlink functionality\n");
  // Create original file
  printf(1, "Creating original file...\n");
  if((fd = open("original.txt", O_CREATE | O_RDWR)) < 0){
    printf(1, "Failed to create original file\n");
    exit();
  }
  write(fd, "Hello from original file!", 24);
  close(fd);

  // Create symlink to existing file
  printf(1, "Creating symbolic link to existing file...\n");
  if(symlink("original.txt", "link1") < 0){
    printf(1, "Failed to create symlink\n");
    exit();
  }

  // Create symlink to non-existent file (should succeed)
  printf(1, "Creating symlink to non-existent file...\n");
  if(symlink("nonexistent.txt", "link2") < 0){
    printf(1, "Failed to create symlink to non-existent file\n");
    exit();
  }

  printf(1, "\nTest 2: O_NOFOLLOW behavior\n");
  if((fd = open("link1", O_RDONLY | O_NOFOLLOW)) >= 0){
    printf(1, "Error: O_NOFOLLOW didn't prevent symlink following\n");
    close(fd);
    exit();
  }
  printf(1, "O_NOFOLLOW working correctly\n");

  printf(1, "\nTest 3: Recursive symlink handling\n");
  // Create a chain of symlinks
  symlink("link1", "link3");
  printf(1, "Reading through chain of symlinks...\n");
  if((fd = open("link3", O_RDONLY)) < 0){
    printf(1, "Failed to follow symlink chain\n");
    exit();
  }
  memset(buf, 0, sizeof(buf));
  read(fd, buf, sizeof(buf));
  printf(1, "Content through symlink chain: %s\n", buf);
  close(fd);

  printf(1, "\nTest 4: Cyclic symlinks\n");
  // Create a cycle
  symlink("link5", "link4");
  symlink("link4", "link5");
  if(open("link4", O_RDONLY) >= 0){
    printf(1, "Error: Failed to detect symlink cycle\n");
    exit();
  }
  printf(1, "Successfully detected symlink cycle\n");

  printf(1, "\nTest 5: Cleanup\n");
  if(unlink("link1") < 0 || unlink("link2") < 0 || 
     unlink("link3") < 0 || unlink("link4") < 0 || 
     unlink("link5") < 0){
    printf(1, "Failed to unlink symlinks\n");
    exit();
  }
  printf(1, "Successfully cleaned up symlinks\n");

  printf(1, "\nAll tests completed successfully!\n");
  exit();
}