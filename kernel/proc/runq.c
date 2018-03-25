#include "proc.h"
#include "smp.h"
#include "percpu.h"
#include "libk.h"

static void runq_balancer(void)
{
	klog_verbose("runq", "Run queue balancer started for CPU %u\n", percpu_get(id));
}

// Needs to be called once per CPU
void runq_init(void)
{
	struct runq *rq = kmalloc(sizeof(struct runq), KM_NONE);
	memset(rq, 0, sizeof *rq);
	percpu_set(run_queue, rq);
}

// Needs to be called once per CPU
void runq_start_balancer(void)
{
	// Start the idle task
	struct task *idle = task_fork(NULL, idle_task, TASK_KTHREAD, NULL);
	cpuset_clear(&idle->affinity);
	cpuset_set_id(&idle->affinity, percpu_get(id), 1);
	cpuset_pin(&idle->affinity);
	idle->state = TASK_RUNNABLE;
	dlist_set_next(idle, list, (struct task *)NULL);
	percpu_get(run_queue)->idle = idle;
	klog("runq", "Idle task at %p\n", idle);

	// Start the run queue balancer
	struct task *balancer = task_fork(NULL, runq_balancer, TASK_KTHREAD, NULL);
	cpuset_clear(&balancer->affinity);
	cpuset_set_id(&balancer->affinity, percpu_get(id), 1);
	cpuset_pin(&balancer->affinity);
	task_wakeup(balancer);
}

void runq_add(struct task *t)
{
	struct runq *run_queue = percpu_get(run_queue);
	spin_lock(&run_queue->lock);

	if (run_queue->head == NULL) {
		run_queue->head = t;
		dlist_set_next(t, list, (struct task *)NULL);
		dlist_set_next(percpu_get(current), list, t);
		t->list.prev = NULL;
	} else
		dlist_append(run_queue->head, list, t);

	spin_unlock(&run_queue->lock);
	//klog("runq", "Added task at %p\n", t);
}

struct task *runq_next(void)
{
	struct runq *run_queue = percpu_get(run_queue);
	spin_lock(&run_queue->lock);

	struct task *t = percpu_get(current);
	struct task *next = dlist_get_next(t, list);
	if (next == NULL) {
		next = percpu_get(run_queue)->head;
		if (next == NULL)
			next = run_queue->idle;
	}

	spin_unlock(&run_queue->lock);
	return next;
}

void runq_remove(struct task *t)
{
	struct runq *run_queue = percpu_get(run_queue);
	spin_lock(&run_queue->lock);
	
	dlist_foreach(cur, list, run_queue->head) {
		if (cur == t) {
			struct task *prev = dlist_get_prev(cur, list);
			struct task *next = dlist_get_next(cur, list);
			if (prev == NULL) {
				run_queue->head = next;
				if (next != NULL)
					next->list.prev = NULL;
			} else {
				dlist_set_next(prev, list, next);
			}
			kprintf("Prev = %p, next = %p\n", prev, next);
			break;
		}
	}
	klog("runq", "Removed task %p\n", t);

	spin_unlock(&run_queue->lock);
}
