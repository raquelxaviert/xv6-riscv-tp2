#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "pstat.h"

extern struct proc proc[NPROC];

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_settickets(void)
{
  int n;
  argint(0, &n);
  if(n < 1)
    return -1;
  myproc()->tickets = n;
  return 0;
}

uint64
sys_getpinfo(void)
{
  uint64 addr;
  struct proc *p;
  struct pstat ps;
  int i;

  argaddr(0, &addr);
  if(addr == 0)
    return -1;

  // Fill in the pstat structure
  for(i = 0, p = proc; p < &proc[NPROC]; p++, i++) {
    acquire(&p->lock);
    if(p->state != UNUSED) {
      ps.inuse[i] = 1;
      ps.tickets[i] = p->tickets;
      ps.pid[i] = p->pid;
      ps.ticks[i] = p->ticks;
    } else {
      ps.inuse[i] = 0;
      ps.tickets[i] = 0;
      ps.pid[i] = 0;
      ps.ticks[i] = 0;
    }
    release(&p->lock);
  }

  // Copy the pstat structure to user space
  if(copyout(myproc()->pagetable, addr, (char *)&ps, sizeof(ps)) < 0)
    return -1;
  return 0;
}
