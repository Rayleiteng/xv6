#include "types.h"
#include "stat.h"
#include "user.h"

lock_t lock;
int counter = 0;

void thread(void *arg1, void *arg2){
    for(int i = 0; i<10; i++){
        int tid = gettid();
        lock_acquire(&lock);
        counter++;
        printf(1,"The counter now is: %d, raised by tid %d.\n", counter, tid);
        lock_release(&lock);
    }
    exit();
}

int main(void){
    lock_init(&lock);
    printf(1,"The counter is: %d at the begining.\n", counter);

    printf(1,"We create 10 threads to do increase the counter by 10 times for each thread.\n");
    for(int i = 0; i<10; i++){
        int tid = thread_create(thread,0,0);
        if(tid == -1){
            lock_acquire(&lock);
            printf(1,"Fail to creat thread.\n");
            lock_release(&lock);
            exit();
        }
    }
    
    for(int i = 0; i<10; i++){
        int tid = thread_join();
        if(tid == -1){
            lock_acquire(&lock);
            printf(1,"Fail to join.\n");
            lock_release(&lock);
            exit();
        }
    }
    printf(1,"At the end, counter is %d, and it should be 100.\n",counter);
    exit();
}