#include "types.h"
#include "user.h"
#include "stat.h"

int
main(int argc, char *argv[])
{
  int ticks;
  if (argc != 2) {
    printf(1, "Usage: sleep ticks\n");
    exit();
  }
  ticks = atoi(argv[1]);
  
  if (ticks <= 0) {
    printf(1, "Error: invalid ticks\n");
    exit();
  }

  printf(1, "(nothing happens for a little while)\n");
  sleep(ticks);
  exit();
}
