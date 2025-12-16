#include "types.h"
#include "user.h"

void	worker(void *arg1, void *arg2)
{
	printf(1, "Worker: arg1=%d, arg2=%d\n", (int)arg1, (int)arg2);
	sleep(50); // 模拟工作
	printf(1, "Worker: exiting\n");
	texit();
	;
}

int	main(void)
{
	void *stack;
	void *join_stack;
	int tid;

	stack = malloc(4096);
	// 注意：通常 clone 传入栈顶，free 传入栈底
	// 假设我们之前约定的 clone 传入的是栈顶 (stack + 4096)
	// 那么 clone 内部记录的 ustack 应该是这个传入的值

	printf(1, "Main: creating thread...\n");
	tid = clone(worker, (void *)10, (void *)20, stack + 4096);

	printf(1, "Main: created thread %d, waiting via join...\n", tid);

	int child_tid = join(&join_stack);

	printf(1, "Main: joined thread %d\n", child_tid);

	// 检查 join 返回的栈地址是否正确
	if (join_stack == stack + 4096)
	{
		printf(1, "Stack address check: PASS\n");
		// 释放栈内存 (注意 malloc 返回的是 stack，不是 stack+4096)
		// 我们需要根据偏移量算回 malloc 的指针，或者你在 clone 时就应该存 stack 而不是 stack+4096
		// 按照 Lab 通用做法，这里我们为了演示先不 free，或者手动减回去
		free(stack);
	}
	else
	{
		printf(1, "Stack address check: FAIL (got %p, expected %p)\n",
			join_stack, stack + 4096);
	}

	texit();
	;
}