#include "types.h"
#include "user.h"

int sjf_job_length(int pid); 

int main(int argc, char *argv[])
{
  int pid = atoi(argv[1]);
  int job_length = sjf_job_length(pid);

  if (job_length < 0)
    printf(1, "Process not found or not schedulable\n");
  else
    printf(1, "Process %d has predicted job length: %d\n", pid, job_length);

  exit();
}
