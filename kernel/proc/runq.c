#include "proc.h"
#include "smp.h"
#include "percpu.h"
#include "libk.h"

#define SCHED_ENTITY(n) rb_entry((n), struct sched_entity, node)

static inline uint64_t min_vruntime(struct runq *r)
{
	if (r->tree.most_left == NULL)
		return 0;
	return SCHED_ENTITY(r->tree.most_left)->vruntime;
}

static inline struct task *node_to_task(struct rb_node *node)
{
	struct sched_entity *sched = rb_entry(node, struct sched_entity, node);
	struct task *t = sched == NULL ? NULL : container_of(sched, struct task, sched);
	return t;
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
	idle->flags = TASK_NEED_PREEMPT; // This is always set
	rq->idle = idle;
}

static inline void tree_insert(struct runq *rq, struct task *t)
{
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
	rq->num_threads++;
}

static inline void tree_remove(struct runq *rq, struct task *t)
{
	rb_erase(&rq->tree, &t->sched.node);
	rq->num_threads--;
}

void runq_add(struct task *t)
{
	struct runq *rq = percpu_get(run_queue);
	spin_lock(&rq->lock);

	// If this hasn't run in a while, set the vruntime to the min_vruntime
	if (t->state == TASK_NOT_STARTED || t->state == TASK_BLOCKED)
		t->sched.vruntime = min_vruntime(rq);

	tree_insert(rq, t);

	spin_unlock(&rq->lock);
}

// Pops the next task from the rbtree
struct task *runq_next(void)
{
	struct runq *rq = percpu_get(run_queue);

	spin_lock(&rq->lock);
	struct rb_node *node = rb_first_cached(&rq->tree);
	struct task *next = node_to_task(node);
	if (next != NULL)
		tree_remove(rq, next);
	spin_unlock(&rq->lock);


	if (next == NULL)
		next = rq->idle;

	return next;
}

void runq_remove(struct task *t)
{
	struct runq *rq = percpu_get(run_queue);
	spin_lock(&rq->lock);
	tree_remove(rq, t);
	spin_unlock(&rq->lock);

	// TODO: Panic if not in tree
	//panic("Tried to remove task at %p that isn't in run queue", t);
}

void runq_balance_pull(void)
{
	struct runq *rq = percpu_get(run_queue);

	for (size_t i = 0; i < smp_nr_cpus(); i++) {
		if (i != smp_cpu_id()) {
			struct runq *other = percpu_table[i]->run_queue;
			bool should_exit = false;

			// Prevent deadlocks by always acquiring the lowest ID's lock first
			bool this_lower_id = smp_cpu_id() < i;
			if (this_lower_id) {
				spin_lock(&rq->lock);	
				spin_lock(&other->lock);
			} else {
				spin_lock(&other->lock);
				spin_lock(&rq->lock);	
			}

			if (other->num_threads > 0) {
				// Pull thread
				// TODO: Don't always pull the first thread
				// TODO: Take into account affinity
				struct task *pulled = node_to_task(rb_first_cached(&other->tree));
				tree_remove(other, pulled);
				tree_insert(rq, pulled);
				should_exit = true;
			}

			// Release locks in correct order
			if (this_lower_id) {
				spin_unlock(&other->lock);
				spin_unlock(&rq->lock);
			} else {
				spin_unlock(&rq->lock);
				spin_unlock(&other->lock);
			}

			if (should_exit)
				break;
		}
	}
}
