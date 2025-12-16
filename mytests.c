#include "types.h"
#include "stat.h"
#include "user.h"

lock_t mylock;
int counter = 0;

void
worker_inc(void *arg1, void *arg2)
{
  int i;
  int n = (int)arg1;
  
  for(i = 0; i < n; i++){
    lock_acquire(&mylock);
    counter++;
    lock_release(&mylock);
  }
  texit();
}

void
test_lock(void)
{
  int i;
  int nthreads = 4;
  int niter = 10000;

  printf(1, "\n=== Test 1: Ticket Lock Race Condition ===\n");
  
  lock_init(&mylock);
  counter = 0;

  printf(1, "Spawning %d threads, each incrementing %d times...\n", nthreads, niter);

  for(i = 0; i < nthreads; i++){
    if(thread_create(worker_inc, (void*)niter, 0) < 0){
      printf(1, "thread_create failed\n");
      exit();
    }
  }

  for(i = 0; i < nthreads; i++){
    thread_join();
  }

  int expected = nthreads * niter;
  if(counter == expected){
    printf(1, "PASS: Counter = %d (Expected %d)\n", counter, expected);
  } else {
    printf(1, "FAIL: Counter = %d (Expected %d). Locks are not working!\n", counter, expected);
  }
}

void
worker_malloc(void *arg1, void *arg2)
{
  int i;
  for(i = 0; i < 100; i++){
    void *p = malloc(1000);
    if(p == 0){
      printf(1, "malloc failed\n");
      texit();
    }
    memset(p, 0xAA, 1000);
    free(p);
  }
  texit();
}

void
test_malloc(void)
{
  int i;
  int nthreads = 4;

  printf(1, "\n=== Test 2: Malloc (sbrk) Concurrency ===\n");
  printf(1, "Spawning %d threads to stress test malloc/free...\n", nthreads);

  for(i = 0; i < nthreads; i++){
    if(thread_create(worker_malloc, 0, 0) < 0){
      printf(1, "thread_create failed\n");
      exit();
    }
  }

  for(i = 0; i < nthreads; i++){
    thread_join();
  }

  printf(1, "PASS: Malloc stress test finished without crash.\n");
}

int
main(int argc, char *argv[])
{
  test_lock();
  test_malloc();
  exit();
}