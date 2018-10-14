#include "proc.h"
#include "smp.h"
#include "percpu.h"
#include "libk.h"

#define SCHED_ENTITY(n) rb_entry((n), struct sched_entity, node)

static uint64_t min_vruntime(struct runq *r)
{
	if (r->tree.most_left == NULL)
		return 0;
	return SCHED_ENTITY(r->tree.most_left)->vruntime;
}

// Initialises the run queue for the current CPU
void runq_init(struct task *initial_parent)
{
	struct runq *rq = kmalloc(sizeof(struct runq), KM_NONE);
	memset(rq, 0, sizeof *rq);
	percpu_set(run_queue, rq);

	// Create the idle task
	struct task *idle = task_fork(initial_parent, idle_task, TASK_KTHREAD, NULL);
	cpuset_clear(&idle->affinity);
	cpuset_set_id(&idle->affinity, percpu_get(id), 1);
	cpuset_pin(&idle->affinity);
	idle->state = TASK_RUNNABLE;
	idle->tid = 0; // Idle task always has ID 0
	idle->tgid = 0;
	dlist_set_next(idle, list, (struct task *)NULL);
	rq->idle = idle;
}

void runq_add(struct task *t)
{
	struct runq *rq = percpu_get(run_queue);
	spin_lock(&rq->lock);

	// If this hasn't run in a while, set the vruntime to the min_vruntime
	if (t->state == TASK_NOT_STARTED || t->state == TASK_BLOCKED)
		t->sched.vruntime = min_vruntime(rq);

	struct sched_entity *first = rb_entry(rb_first_cached(&rq->tree), struct sched_entity, node);

	// Find the right nodes to link with
	struct rb_node **link = &rq->tree.root, *parent = NULL;
	while (*link) {
		struct sched_entity *this = rb_entry(*link, struct sched_entity, node);
		parent = *link;

		if (t->sched.vruntime < this->vruntime)
			link = &(*link)->left;
		else
			link = &(*link)->right;
	}

	// Add to the rbtree
	rb_link_node(&t->sched.node, parent, link);
	rb_insert(&rq->tree, &t->sched.node, first == NULL || t->sched.vruntime < first->vruntime);

	spin_unlock(&rq->lock);
}

// Pops the next task from the rbtree
struct task *runq_next(void)
{
	struct runq *rq = percpu_get(run_queue);

	spin_lock(&rq->lock);
	struct rb_node *node = rb_first_cached(&rq->tree);
	if (node != NULL)
		rb_erase(&rq->tree, node);
	spin_unlock(&rq->lock);

	struct sched_entity *se = rb_entry(node, struct sched_entity, node);
	struct task *next = se == NULL ? NULL : container_of(se, struct task, sched);

	if (next == NULL)
		next = rq->idle;

	return next;
}

void runq_remove(struct task *t)
{
	struct runq *rq = percpu_get(run_queue);
	spin_lock(&rq->lock);
	rb_erase(&rq->tree, &t->sched.node);
	spin_unlock(&rq->lock);

	// TODO: Panic if not in tree
	//panic("Tried to remove task at %p that isn't in run queue", t);
}

void runq_balance_pull(void)
{
	struct runq *rq = percpu_get(run_queue);
	spin_lock(&rq->lock);
	// TODO: Implement
	spin_unlock(&rq->lock);
}
