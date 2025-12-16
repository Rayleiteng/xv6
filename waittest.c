#include "types.h"
#include "stat.h"
#include "user.h"

// 简单的线程工作函数
void	worker(void *arg1, void *arg2)
{
	int	id;

	id = (int)arg1;
	sleep(10); // 睡一会儿，让主线程有机会先调用 wait
	printf(1, "  [Thread %d] working...\n", id);
	texit(); // 必须用 texit!
}

void	test_wait_ignore_threads(void)
{
	void	*stack;
	int		tid;
	int		ret;
	void	*join_stack;

	printf(1, "\n=== Test A: Only Threads ===\n");
	stack = malloc(4096);
	tid = clone(worker, (void *)111, 0, stack + 4096);
	printf(1, "Parent: Created thread %d. Calling wait() now...\n", tid);
	// 【关键点】
	// 因为没有 fork 出的子进程，wait() 应该立刻返回 -1。
	// 如果你的 wait() 写错了，它可能会阻塞在这里等待线程，或者返回 tid。
	ret = wait();
	if (ret == -1)
	{
		printf(1, "PASS: wait() returned -1 (Correctly ignored the thread)\n");
	}
	else if (ret == tid)
	{
		printf(1, "FAIL: wait() returned %d (It mistakenly waited for a thread!)\n", ret);
	}
	else
	{
		printf(1, "FAIL: wait() returned unexpected %d\n", ret);
	}
	// 别忘了回收线程，否则内存泄露
	join(&join_stack);
	free(stack);
	printf(1, "Parent: Joined thread %d.\n", tid);
}

void	test_mixed_family(void)
{
	void	*stack;
	int		tid;
	int		pid;
	int		caught;
	void	*join_stack;

	printf(1, "\n=== Test B: Thread + Process Mixed ===\n");
	stack = malloc(4096);
	// 1. 先造一个线程
	tid = clone(worker, (void *)222, 0, stack + 4096);
	printf(1, "Parent: Created thread %d\n", tid);
	// 2. 再造一个真·子进程
	pid = fork();
	if (pid == 0)
	{
		printf(1, "  [Child Process] I am a process, exiting now.\n");
		exit();
	}
	printf(1, "Parent: Created child process %d\n", pid);
	// 3. 调用 wait()
	// 预期：必须返回 pid，绝对不能返回 tid
	printf(1, "Parent: Calling wait()...\n");
	caught = wait();
	if (caught == pid)
	{
		printf(1, "PASS: wait() caught child process %d\n", caught);
	}
	else if (caught == tid)
	{
		printf(1, "FAIL: wait() caught thread %d! (Should ignore threads)\n",
			caught);
	}
	else
	{
		printf(1, "FAIL: wait() returned %d\n", caught);
	}
	// 4. 清理
	join(&join_stack);
	free(stack);
}

int	main(int argc, char *argv[])
{
	test_wait_ignore_threads();
	test_mixed_family();
	exit();
}