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
	struct task *idle = create_kthread(idle_task, NULL);
	cpuset_clear(&idle->affinity);
	cpuset_set_id(&idle->affinity, percpu_get(id), 1);
	cpuset_pin(&idle->affinity);
	idle->state = TASK_RUNNABLE;
	dlist_set_next(idle, list, (struct task *)NULL);
	percpu_get(run_queue)->idle = idle;

	// Start the run queue balancer
	struct task *balancer = create_kthread(runq_balancer, NULL);
	cpuset_clear(&balancer->affinity);
	cpuset_set_id(&balancer->affinity, percpu_get(id), 1);
	cpuset_pin(&balancer->affinity);
	task_wakeup(balancer);
}

void runq_add(struct task *t)
{
	preempt_inc();
	struct runq *run_queue = percpu_get(run_queue);
	spin_lock(&run_queue->lock);
	preempt_dec();

	if (run_queue->head == NULL) {
		run_queue->head = t;
		dlist_set_next(t, list, (struct task *)NULL);
		dlist_set_next(current, list, t);
		t->list.prev = NULL;
	} else
		dlist_append(run_queue->head, list, t);

	spin_unlock(&run_queue->lock);
	klog_verbose("runq", "Added task at %p\n", t);
}

struct task *runq_next(void)
{
	preempt_inc();
	struct runq *run_queue = percpu_get(run_queue);
	spin_lock(&run_queue->lock);
	preempt_dec();

	struct task *t = current;
	struct task *next = dlist_get_next(t, list);
	if (next == NULL) {
		next = run_queue->head;
		if (next == NULL)
			next = run_queue->idle;
	} else
		kassert_dbg(next != current);

	spin_unlock(&run_queue->lock);
	klog_verbose("runq", "Next task is %p, then %p\n", next, next->list.next);
	return next;
}

void runq_remove(struct task *t)
{
	preempt_inc();
	struct runq *run_queue = percpu_get(run_queue);
	spin_lock(&run_queue->lock);
	preempt_dec();
	
	dlist_foreach(cur, list, run_queue->head) {
		if (cur == t) {
			struct task *prev = dlist_get_prev(cur, list);
			struct task *next = dlist_get_next(cur, list);

			if (run_queue->head == cur)
				run_queue->head = next;

			if (prev != NULL)
				dlist_set_next(prev, list, next);
			if (next != NULL)
				dlist_set_prev(next, list, prev);

			spin_unlock(&run_queue->lock);
			klog_verbose("runq", "Deleting %p with prev %p, next %p\n", t, prev, next);
			klog_verbose("runq", "Removed task %p\n", t);
			return;
		}
	}

	panic("Tried to remove task at %p that isn't in run queue", t);
}
