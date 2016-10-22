#include "sched.h"

//runqueue_t *colas;
//int *quantoNivel;

	int q0=2;
	int q1=4;
	int q2=6;
struct mlq_data{
	int nivelCola;
	int remaining_ticks_slice;
};

struct mlq_data_rq{
	int q0;
	int q1;
	int q2;
};

static int sched_init(runqueue_t* rq){
	struct mlq_data_rq* rq_cs_data = malloc(sizeof(struct mlq_data_rq));
	rq_cs_data->q0 = 2;
	rq_cs_data->q1 = 4;
	rq_cs_data->q2 = 6;

	rq->rq_cs_data = rq_cs_data;

	return 0;
}

static int compare_tasks_nivel(void *t1, void *t2) {
	task_t* tsk1 = (task_t*) t1;
	task_t* tsk2 = (task_t*) t2;

	struct mlq_data* cs_data1 = (struct mlq_data*) tsk1->tcs_data;
	struct mlq_data* cs_data2 = (struct mlq_data*) tsk2->tcs_data;


	return cs_data1->nivelCola - cs_data2->nivelCola;
}

static void sched_destroy(runqueue_t* rq){
	if (rq->rq_cs_data) {
		free(rq->rq_cs_data);
		rq->rq_cs_data = NULL;
	}
}
static int task_new_mlq(task_t* t) {
	struct mlq_data* cs_data = malloc(sizeof(struct mlq_data));

	if (!cs_data)
		return 1; /* Cannot reserve memory */

	/* initialize the quantum */
	cs_data->nivelCola = 0;
	cs_data->remaining_ticks_slice = 2;
	t->tcs_data = cs_data;

	return 0;
}
static void task_free_mlq(task_t* t) {
	if (t->tcs_data) {
		free(t->tcs_data);
		t->tcs_data = NULL;
	}
}
static task_t* pick_next_task_mlq(runqueue_t* rq, int cpu) {
	task_t* t = head_slist(&rq->tasks); //List sorted by ORDEN DE LLEGADA (just pick the first one).

	if (t) {
		/* Current is not on the rq*/
		remove_slist(&rq->tasks, t);
		t->on_rq = FALSE;
		rq->cur_task = t;
	}

	return t;
}

static void enqueue_task_mlq(task_t* t, int cpu, int runnable) { //MIRAR ESTO--> SI HAY CPU LIBRES DEBERIAN EMPEZARLO LA OTRA
	runqueue_t* rq = get_runqueue_cpu(cpu);

	if (t->on_rq || is_idle_task(t))
		return;

	if (t->flags & TF_INSERT_FRONT) {
		//Clear flag
		t->flags &= ~TF_INSERT_FRONT;
		sorted_insert_slist_front(&rq->tasks, t, 1, compare_tasks_nivel); //Push task
	} else
		sorted_insert_slist(&rq->tasks, t, 1, compare_tasks_nivel);  //Push task

	t->on_rq = TRUE;

	/* If the task was not runnable before, update the number of runnable tasks in the rq*/
	/* Si la tarea no estaba ejecutandose anteriormente -> aumentamos numero procesos de la cola de la CPU */
	if (!runnable) {
		rq->nr_runnable++;
		t->last_cpu = cpu;

		/* Trigger a preemption if this task has a shorter CPU burst than current */
/*		if (!is_idle_task(current) && t->prio < current->prio) {
			rq->need_resched = TRUE;
			current->flags |= TF_INSERT_FRONT;  To avoid unfair situations in the event
			 another task with the same prio wakes up as well
		}*/
	}
}

static void task_tick_mlq(runqueue_t* rq, int cpu) {

	task_t* current = rq->cur_task;
	struct mlq_data* cs_data = (struct mlq_data*) current->tcs_data;

	if (is_idle_task(current))
		return;

	cs_data->remaining_ticks_slice--; /* Charge tick */

	if (cs_data->remaining_ticks_slice <= 0){
		rq->need_resched = TRUE; //Force a resched !!
		if(cs_data->nivelCola < 2){
			cs_data->nivelCola++;

			if(cs_data->nivelCola == 1){
				cs_data->remaining_ticks_slice = q1;
			}
			if(cs_data->nivelCola == 2){
				cs_data->remaining_ticks_slice = q2;
			}


		}
	}

	if (current->runnable_ticks_left == 1){
		rq->nr_runnable--; // The task is either exiting or going to sleep right now
	}
}

static task_t* steal_task_mlq(runqueue_t* rq, int cpu) {
	task_t* t = tail_slist(&rq->tasks);

	if (t) {
		remove_slist(&rq->tasks, t);
		t->on_rq = FALSE;
		rq->nr_runnable--;
	}

	return t;
}

sched_class_t mlq_sched = { .pick_next_task = pick_next_task_mlq,
		.enqueue_task = enqueue_task_mlq, .task_tick = task_tick_mlq,
		.steal_task = steal_task_mlq, .sched_init = sched_init, .sched_destroy = sched_destroy, .task_new = task_new_mlq, .task_free = task_free_mlq};
