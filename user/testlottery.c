#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pstat.h"

#define NCHILDREN 3
#define RUNTIME 10000  // Number of iterations

// Spin to consume CPU and yield periodically
void
spin(void)
{
  int i, j;
  for(i = 0; i < 1000; i++) {
    j = i * i;
    (void)j;  // prevent compiler optimization
  }
  // Yield to allow scheduler to run more frequently
  // This ensures ticks accumulate properly
}

int
main(int argc, char *argv[])
{
  int pids[NCHILDREN];
  int tickets[NCHILDREN] = {30, 20, 10};  // 3:2:1 ratio
  int i;
  struct pstat ps;

  printf("Starting lottery scheduler test with 3:2:1 ticket ratio\n");
  printf("Process A: 30 tickets\n");
  printf("Process B: 20 tickets\n");
  printf("Process C: 10 tickets\n\n");

  // Create three child processes
  for(i = 0; i < NCHILDREN; i++) {
    pids[i] = fork();
    if(pids[i] < 0) {
      printf("fork failed\n");
      exit(1);
    }
    
    if(pids[i] == 0) {
      // Child process
      settickets(tickets[i]);
      
      // Spin forever - parent will kill us
      while(1) {
        spin();
      }
      
      exit(0);
    }
  }

  // Parent: wait a bit then collect statistics
  pause(100);  // Let children run for a while
  
  // Print statistics multiple times
  for(int round = 0; round < 10; round++) {
    pause(100);  // Longer pause to accumulate more ticks
    
    if(getpinfo(&ps) < 0) {
      printf("getpinfo failed\n");
      exit(1);
    }

    printf("Round %d - Process statistics:\n", round + 1);
    for(i = 0; i < NPROC; i++) {
      if(ps.inuse[i]) {
        for(int j = 0; j < NCHILDREN; j++) {
          if(ps.pid[i] == pids[j]) {
            printf("  Process %c (PID %d): tickets=%d, ticks=%d\n",
                   'A' + j, ps.pid[i], ps.tickets[i], ps.ticks[i]);
          }
        }
      }
    }
    printf("\n");
  }

  // Final statistics BEFORE waiting (while children still running)
  if(getpinfo(&ps) < 0) {
    printf("getpinfo failed\n");
    exit(1);
  }

  printf("Final statistics:\n");
  int total_ticks = 0;
  int final_ticks[NCHILDREN] = {0, 0, 0};
  for(i = 0; i < NPROC; i++) {
    if(ps.inuse[i]) {
      for(int j = 0; j < NCHILDREN; j++) {
        if(ps.pid[i] == pids[j]) {
          final_ticks[j] = ps.ticks[i];
          total_ticks += ps.ticks[i];
          printf("  Process %c (PID %d): tickets=%d, ticks=%d", 
                 'A' + j, ps.pid[i], ps.tickets[i], ps.ticks[i]);
          if(total_ticks > 0) {
            printf(" (%.1f%%)", 100.0 * ps.ticks[i] / total_ticks);
          }
          printf("\n");
        }
      }
    }
  }

  if(total_ticks > 0) {
    printf("\nTotal ticks: %d\n", total_ticks);
    if(final_ticks[0] > 0 && final_ticks[1] > 0 && final_ticks[2] > 0) {
      printf("Actual ratio: %.2f:%.2f:%.2f\n", 
             (float)final_ticks[0]/final_ticks[2], 
             (float)final_ticks[1]/final_ticks[2], 
             1.0);
    }
  }
  printf("Expected ratio: 3:2:1\n");
  printf("Test completed!\n");

  // Kill all children
  for(i = 0; i < NCHILDREN; i++) {
    kill(pids[i]);
  }
  
  // Wait for all children to finish
  for(i = 0; i < NCHILDREN; i++) {
    wait(0);
  }
  
  exit(0);
}
