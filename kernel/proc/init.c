#include "proc.h"
#include "syscall.h"
#include "libk.h"
#include "fs.h"

static void init_user(void);

// The first kernel thread. Perform advanced initialisation (e.g forking) from here.
// Ends with a call to execve, beginning the first user process.
void init_kernel(void)
{
	vfs_init();

	struct inode *out;
	err_t e;
	if ((e = vfs_lookup(&vfs_root, "/", 2, &out)))
		panic("Failed to find root directory: %s", strerror(e));

	inode_put(out);

	task_execve(init_user, NULL, 0);
}

static void init_user(void)
{
	int x = 0;
	if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, 'A' + (x++ % 26), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, 'a' + (x++ % 26), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, '0' + (x++ % 10), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, 'A' + (x++ % 26), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, 'a' + (x++ % 26), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, '0' + (x++ % 10), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, 'A' + (x++ % 26), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, 'a' + (x++ % 26), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else {
		while (1)
			;//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
	}
}
