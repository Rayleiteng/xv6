#include "types.h"
#include "user.h"
#include "stat.h"

#define THREAD_COUNT 4      // 线程数量
#define ITERATIONS 100      // 每个线程申请多少次内存
#define CHUNK_SIZE 100      // 每次申请多少字节

// 全局计数器，记录发生了多少次冲突
volatile int error_count = 0;

void worker(void *arg1, void *arg2) {
    int id = (int)arg1; // 当前线程的ID (1, 2, 3, 4)
    int i, j;
    char *mem;

    for (i = 0; i < ITERATIONS; i++) {
        // 1. 申请内存 (底层调用 sbrk)
        // 如果 sbrk 没锁，两个线程可能拿到相同的 mem 指针
        mem = (char *)malloc(CHUNK_SIZE);

        if (mem == 0) {
            printf(1, "Thread %d: malloc failed\n", id);
            break;
        }

        // 2. 写入数据：把这块内存全部填满当前线程的 ID
        for (j = 0; j < CHUNK_SIZE; j++) {
            mem[j] = (char)id;
        }

        // 3. 主动让出 CPU (sleep)，增加别的线程来捣乱的概率
        sleep(1);

        // 4. 检查数据：看看是不是被别的线程覆盖了
        for (j = 0; j < CHUNK_SIZE; j++) {
            if (mem[j] != (char)id) {
                // 抓到了！数据不一致！
                printf(1, "RACE DETECTED! Thread %d addr %x: expected %d got %d\n", 
                       id, mem, id, mem[j]);
                __sync_fetch_and_add(&error_count, 1);
                goto done; // 发现错误就退出当前次循环
            }
        }
        
        // 这里的 free 不是必须的，不 free 更容易让 sbrk 持续增长，增加压力
        // free(mem); 
    }

done:
    exit();
}

int main(int argc, char *argv[]) {
    int i;
    printf(1, "Starting sbrk race test (sbrk_race)...\n");

    for (i = 1; i <= THREAD_COUNT; i++) {
        // 这里的 thread_create 是你之前实现的
        if (thread_create(worker, (void*)i, 0) < 0) {
            printf(1, "Error creating thread %d\n", i);
        }
    }

    // 等待所有子线程结束
    for (i = 1; i <= THREAD_COUNT; i++) {
        thread_join();
    }

    if (error_count == 0) {
        printf(1, "PASSED: No race conditions detected.\n");
    } else {
        printf(1, "FAILED: Detected %d race conditions!\n", error_count);
        printf(1, "Hint: Your sys_sbrk is not thread-safe.\n");
    }

    exit();
}