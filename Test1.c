#include "types.h"
#include "stat.h"
#include "user.h"

lock_t print_lock;

void grandchild_worker(void *arg1, void *arg2)
{    
    int id = (int)arg1;
    lock_acquire(&print_lock);
    printf(1, "    [Grandchild %d] Running in Child Process (PID %d)...\n", id, getpid());
    lock_release(&print_lock);
    sleep(20); 
    lock_acquire(&print_lock);
    printf(1, "    [Grandchild %d] Done work. Exiting (becomes ZOMBIE).\n", id);
    lock_release(&print_lock);
    exit(); 
}

void forker_thread(void *arg1, void *arg2)
{
    int pid;
    void *stack1, *stack2;
    lock_acquire(&print_lock);
    printf(1, "  [Forker] I am a thread in Parent (PID %d). Calling fork() now...\n", getpid());
    lock_release(&print_lock);
    pid = fork();
    if (pid < 0) {
        lock_acquire(&print_lock);
        printf(1, "  [Forker] Fork failed!\n");
        lock_release(&print_lock);
        exit();
    }
    if (pid == 0) {
        lock_init(&print_lock); 
        lock_acquire(&print_lock);
        printf(1, "  [Child Main] I am the Child Process (PID %d).\n", getpid());
        printf(1, "  [Child Main] Note: I inherited the stack/context of the Forker thread.\n");
        lock_release(&print_lock);
        stack1 = malloc(4096);
        stack2 = malloc(4096);
        lock_acquire(&print_lock);
        printf(1, "  [Child Main] Creating 2 grandchild threads...\n");
        lock_release(&print_lock);
        clone(grandchild_worker, (void*)1, 0, stack1 + 4096);
        clone(grandchild_worker, (void*)2, 0, stack2 + 4096);
        lock_acquire(&print_lock);
        printf(1, "  [Child Main] Calling exit() immediately (Should wait for grandchildren).\n");
        lock_release(&print_lock);
        exit(); 
    } 
    else {
        lock_acquire(&print_lock);
        printf(1, "  [Forker] Fork success. Child PID is %d. I will wait() for it.\n", pid);
        lock_release(&print_lock);
        wait();
        lock_acquire(&print_lock);
        printf(1, "  [Forker] Child %d has exited correctly.\n", pid);
        printf(1, "  [Forker] My job is done. Exiting.\n");
        lock_release(&print_lock);
        exit();
    }
}

void simple_worker(void *arg1, void *arg2)
{
    lock_acquire(&print_lock);
    printf(1, "  [Worker] I am a simple sibling of Forker. (PID %d)\n", getpid());
    lock_release(&print_lock);
    sleep(10);
    lock_acquire(&print_lock);
    printf(1, "  [Worker] Exiting.\n");
    lock_release(&print_lock);
    exit();
}

int main(void)
{
    int tid_forker, tid_worker;
    int joined_id;
    lock_init(&print_lock);
    lock_acquire(&print_lock);
    printf(1, "\n=== Complex Fork & Thread Test ===\n");
    printf(1, "[Main] PID %d. Creating threads...\n", getpid());
    lock_release(&print_lock);
    tid_forker = thread_create(forker_thread, 0, 0); 
    tid_worker = thread_create(simple_worker, 0, 0); 
    lock_acquire(&print_lock);
    printf(1, "[Main] Created Forker (tid %d) and Worker (tid %d).\n", tid_forker, tid_worker);
    printf(1, "[Main] Waiting for threads to finish...\n");
    lock_release(&print_lock);
    int i;
    for(i = 0; i < 2; i++){
        joined_id = thread_join();
        
        lock_acquire(&print_lock);
        if(joined_id == tid_forker){
            printf(1, "[Main] Forker thread joined.\n");
        } else if(joined_id == tid_worker){
            printf(1, "[Main] Simple worker thread joined.\n");
        } else {
            printf(1, "[Main] Unknown thread joined: %d\n", joined_id);
        }
        lock_release(&print_lock);
    }
    lock_acquire(&print_lock);
    printf(1, "[Main] All threads finished.\n");
    printf(1, "PASS: Test Completed Successfully.\n");
    lock_release(&print_lock);
    exit();
}