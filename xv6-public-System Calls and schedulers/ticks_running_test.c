#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        printf(1, "Usage: ticks_running_test <pid>\n");
        exit();
    }

    int pid = atoi(argv[1]); // Convert argument to an integer (PID)
    int ticks = ticks_running(pid);

    if (ticks == -1) {
        printf(1, "-1 Failed to get ticks for process %d (process does not exist)\n", pid);
    } else if (ticks == 0) {
        printf(1, "0 Process %d has never been scheduled or is unscheduled.\n", pid);
    } else {
        printf(1, "Process %d has ticks: %d\n", pid, ticks);
    }

    exit();
}
