#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

int sjf_job_length(int pid);

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int sys_sbrk(void) {
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  myproc()->sz += n;    // Only update size, removed growproc() call
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_hello(void)
{
  cprintf("Hello from the Kernel!\n");
  return 0;
}
extern struct {
    struct spinlock lock;
    struct proc proc[NPROC];
} ptable;

int
sys_ticks_running(void)
{
    int pid;
    struct proc *p;

    // Retrieve the PID from the system call argument
    if (argint(0, &pid) < 0)
        return -1;

    // Iterate over the process table to find the process with the given pid
    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            // Found the process, now check its state

            // If it's UNUSED or ZOMBIE, return -1 as the process does not exist
            if (p->state == UNUSED || p->state == ZOMBIE) {
                release(&ptable.lock);
                return -1;
            }

            // If the process exists but hasn't been scheduled, return 0
            if (p->ticks == 0) {
                release(&ptable.lock);
                return 0;
            }

            // Otherwise, return the number of ticks the process has been scheduled
            release(&ptable.lock);
            return p->ticks; 
        }
    }

    // If no process with the given PID is found, return -1
    release(&ptable.lock);
    return -1; // Process does not exist
}

int
sys_sjf_job_length(void) {
    int pid;
    struct proc *p;

    if (argint(0, &pid) < 0)
        return -1;

    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if(p->pid == pid) {
            release(&ptable.lock);
            return p->predicted_job_length; // Return job length
        }
    }
    release(&ptable.lock);
    return -1; // Process does not exist
}

int
sys_set_lottery_tickets(void)
{
    int tickets;

    if (argint(0, &tickets) < 0)
        return -1;

    struct proc *p = myproc();  // Get the current process
    p->tickets = tickets;       // Set the number of tickets for the current process

    return 0;
}

int sys_get_lottery_tickets(void) {
    int pid;
    struct proc *p;

    // Retrieve the pid argument from the system call
    if(argint(0, &pid) < 0) {
        return -1;
    }

    // Lock the process table
    acquire(&ptable.lock);

    // Iterate over the process table to find the process with the given pid
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if(p->pid == pid && (p->state == RUNNABLE || p->state == RUNNING)) {
            // Process found, return the number of tickets
            release(&ptable.lock);
            return p->tickets;
        }
    }

    // Process not found, or it's not in the RUNNABLE or RUNNING state
    release(&ptable.lock);
    return -1;
}

