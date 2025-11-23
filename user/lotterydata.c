#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pstat.h"

#define NCHILDREN 3
#define NSAMPLES 50
#define SAMPLE_INTERVAL 20

// Spin to consume CPU
void
spin(void)
{
  int i, j;
  for(i = 0; i < 100000; i++) {
    j = i * i;
    (void)j;
  }
}

int
main(int argc, char *argv[])
{
  int pids[NCHILDREN];
  int tickets[NCHILDREN] = {30, 20, 10};  // 3:2:1 ratio
  int i;
  struct pstat ps;

  // Print CSV header
  printf("sample,process_a,process_b,process_c\n");

  // Create three child processes
  for(i = 0; i < NCHILDREN; i++) {
    pids[i] = fork();
    if(pids[i] < 0) {
      printf("fork failed\n");
      exit(1);
    }
    
    if(pids[i] == 0) {
      // Child process - set tickets and spin forever
      settickets(tickets[i]);
      while(1) {
        spin();
      }
    }
  }

  // Parent: collect samples
  int prev_ticks[NCHILDREN] = {0, 0, 0};
  
  for(int sample = 0; sample < NSAMPLES; sample++) {
    pause(SAMPLE_INTERVAL);
    
    if(getpinfo(&ps) < 0) {
      printf("getpinfo failed\n");
      exit(1);
    }

    int curr_ticks[NCHILDREN];
    
    // Find ticks for each child
    for(i = 0; i < NPROC; i++) {
      if(ps.inuse[i]) {
        for(int j = 0; j < NCHILDREN; j++) {
          if(ps.pid[i] == pids[j]) {
            curr_ticks[j] = ps.ticks[i];
          }
        }
      }
    }

    // Print delta (ticks since last sample)
    printf("%d,%d,%d,%d\n", 
           sample + 1,
           curr_ticks[0] - prev_ticks[0],
           curr_ticks[1] - prev_ticks[1],
           curr_ticks[2] - prev_ticks[2]);

    for(i = 0; i < NCHILDREN; i++) {
      prev_ticks[i] = curr_ticks[i];
    }
  }

  // Kill children
  for(i = 0; i < NCHILDREN; i++) {
    kill(pids[i]);
    wait(0);
  }
  
  exit(0);
}
