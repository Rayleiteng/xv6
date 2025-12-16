#include "types.h"
#include "stat.h"
#include "user.h"

// 子线程函数：故意拖延时间
void
slow_worker(void *arg1, void *arg2)
{
  printf(1, "  [Worker] I am sleeping for 100 ticks...\n");
  sleep(100); // 睡眠约 1 秒
  printf(1, "  [Worker] Done sleeping. Exiting.\n");
  exit(); // 在你的实现中，这会自动转为 texit()
}

int
main(int argc, char *argv[])
{
  int pid;
  int start_time, end_time, duration;

  printf(1, "exittest: Starting test for Main-Thread-Wait logic...\n");

  start_time = uptime(); // 记录开始时间

  pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  }

  if(pid == 0){
    // === 子进程 (Child Process) ===
    
    // 1. 创建一个慢速线程
    void *stack = malloc(4096);
    if(clone(slow_worker, 0, 0, stack + 4096) < 0){
        printf(1, "clone failed\n");
        exit();
    }

    // 2. 主线程不等待，直接调用 exit()
    printf(1, "  [Child Main] Calling exit() immediately.\n");
    printf(1, "  [Child Main] I should be blocked until worker finishes.\n");
    exit(); 
  }

  // === 父进程 (Parent Process) ===
  wait(); // 等待子进程完全退出
  end_time = uptime(); // 记录结束时间

  duration = end_time - start_time;
  printf(1, "exittest: Process exited after %d ticks.\n", duration);

  // 验证结果
  // 如果 exit 逻辑正确，总时间应该 >= 100 (因为子线程睡了 100)
  // 如果 exit 逻辑错误（没等子线程），时间会非常短（比如 0 或 1）
  if(duration >= 90){ // 给一点误差空间
    printf(1, "exittest: PASS (Main thread correctly waited for worker)\n");
  } else {
    printf(1, "exittest: FAIL (Main thread exited too early!)\n");
  }

  exit();
}