#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pstat.h"

int
main(int argc, char *argv[])
{
  struct pstat ps;
  int i;

  if(getpinfo(&ps) < 0) {
    printf("getpinfo failed\n");
    exit(1);
  }

  printf("PID\tTICKETS\tTICKS\n");
  printf("---\t-------\t-----\n");
  
  for(i = 0; i < NPROC; i++) {
    if(ps.inuse[i]) {
      printf("%d\t%d\t%d\n", ps.pid[i], ps.tickets[i], ps.ticks[i]);
    }
  }
  
  exit(0);
}
