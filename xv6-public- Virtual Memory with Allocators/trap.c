#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"     // For struct gatedesc and SETGATE
#include "proc.h"
#include "x86.h"     // For lidt()
#include "traps.h"   // For T_SYSCALL and trap numbers
#include "spinlock.h"

#define DEBUG_ALLOC 1
#if defined(LOCALITY_ALLOCATOR)
#define ALLOCATOR_TYPE "LOCALITY"
#elif defined(LAZY_ALLOCATOR)
#define ALLOCATOR_TYPE "LAZY"
#else
#error "No allocator type defined"
#endif

struct spinlock tickslock;
uint ticks;
struct gatedesc idt[256];
extern uint vectors[];  
  // vectors.S declares these
extern pte_t* walkpgdir(pde_t *pgdir, const void *va, int alloc);
// Move mappages from vm.c to here
static int handle_page_fault(struct proc *curproc, uint va);


// trap.c

void idtinit(void)
{
  lidt(idt, sizeof(idt));
}


int mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
{
  char *a, *last;
  pte_t *pte;

  a = (char*)PGROUNDDOWN((uint)va);
  last = (char*)PGROUNDDOWN(((uint)va) + size - 1);
  for(;;){
    if((pte = walkpgdir(pgdir, a, 1)) == 0)
      return -1;
    if(*pte & PTE_P)
      panic("remap");
    *pte = pa | perm | PTE_P;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}


void
tvinit(void)
{
    int i;
    for(i = 0; i < 256; i++)
        SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
    SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
    initlock(&tickslock, "time");
}

static int
allocate_and_map_pages(struct proc *curproc, uint va)
{
    char *mem;
    uint a;
    int i;
    int pages_to_alloc = 3;  // Allocate 3 pages for locality-aware allocation

    for(i = 0; i < pages_to_alloc; i++) {
        a = PGROUNDDOWN(va) + (i * PGSIZE);
        
        // Don't allocate beyond process size
        if(a >= curproc->sz)
            break;
        
        // Skip if page is already mapped
        pte_t *pte = walkpgdir(curproc->pgdir, (char*)a, 0);
        if(pte && (*pte & PTE_P))
            continue;

        // Allocate new page
        mem = kalloc();
        if(mem == 0) {
            return -1;  // Out of memory
        }
        memset(mem, 0, PGSIZE);
        
        // Map the page
        if(mappages(curproc->pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0) {
            kfree(mem);
            return -1;
        }
    }
    return 0;
}

void
trap(struct trapframe *tf)
{
    
    if(tf->trapno == T_SYSCALL){
        if(myproc()->killed)
            exit();
        myproc()->tf = tf;
        syscall();
        if(myproc()->killed)
            exit();
        return;
    }

    switch(tf->trapno){
    case T_PGFLT:
        if((tf->cs&3) == DPL_USER){  // User space fault
            uint va = rcr2();
            struct proc *curproc = myproc();

            // Check if address is valid
            if(va < curproc->sz && va >= PGSIZE && va < KERNBASE){
                if(handle_page_fault(curproc, va) < 0) {
                    cprintf("out of memory\n");
                    curproc->killed = 1;
                    break;
                }
                return;
            }
        }
    // Fall through if not handled
    
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}




static int
handle_page_fault(struct proc *curproc, uint va)
{
    char *mem;
    uint page_addr = PGROUNDDOWN(va);
    
    // Print allocator type when fault occurs
    cprintf("\nUsing %s allocator\n", ALLOCATOR_TYPE);

    #ifdef LOCALITY_ALLOCATOR
        cprintf("[LOCALITY] Starting allocation of up to 3 pages from 0x%x\n", page_addr);
        int pages_allocated = 0;
        
        for(int i = 0; i < 3; i++) {
            uint curr_addr = page_addr + (i * PGSIZE);
            
            if(curr_addr >= curproc->sz) {
                cprintf("  Stopping: address 0x%x beyond process size 0x%x\n", 
                        curr_addr, curproc->sz);
                break;
            }

            pte_t *pte = walkpgdir(curproc->pgdir, (char*)curr_addr, 0);
            if(pte && (*pte & PTE_P)) {
                cprintf("  Page at 0x%x already mapped\n", curr_addr);
                continue;
            }

            mem = kalloc();
            if(mem == 0) {
                cprintf("  Failed to allocate memory at 0x%x\n", curr_addr);
                return -1;
            }

            memset(mem, 0, PGSIZE);
            if(mappages(curproc->pgdir, (char*)curr_addr, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0) {
                cprintf("  Failed to map page at 0x%x\n", curr_addr);
                kfree(mem);
                return -1;
            }
            
            pages_allocated++;
            cprintf("  Allocated page at 0x%x\n", curr_addr);
        }
        cprintf("[LOCALITY] Allocated %d pages\n", pages_allocated);
    #else
        cprintf("[LAZY] Allocating single page at 0x%x\n", page_addr);
        
        mem = kalloc();
        if(mem == 0) {
            cprintf("  Failed to allocate memory\n");
            return -1;
        }

        memset(mem, 0, PGSIZE);
        if(mappages(curproc->pgdir, (char*)page_addr, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0) {
            cprintf("  Failed to map page\n");
            kfree(mem);
            return -1;
        }
        cprintf("[LAZY] Successfully allocated page\n");
    #endif

    return 0;
}