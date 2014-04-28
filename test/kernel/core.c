/*
 * kernel/core.c
 *
 * This file
 */

#include "os.h"
#include "errno.h"
#include "uart.h"
#include "memory.h"
#include "time.h"


static void init_variable(void);
static void init_tcb_list(void);
static void sched_new(void);
static void change_task_status(struct os_tcb *tcb_ptr);
static void init_idle_task(void);
static void init_stat_task(void);


/**
 * init_os() - Initialize the system.
 * @arg:
 *
 * Return: None
 */
void
init_os(void)
{
	/* Initialize system global variables */
	init_variable();

	/* Initialize the free list of TCBs */
	init_tcb_list();

	/* Initialize idle task */
	init_idle_task();

#if ENABLE_STAT_TASK
	/* Initialize statistic task */
	init_stat_task();
#endif

	/* Initialize the memory list */
	init_mem_list();
}


/**
 *
 * Should just called inside the init_os().
 */
static void
init_variable(void)
{
	/* Clear the 32-bit system clock */
	os_time = 0;

	/* Clear the interrupt nesting counter */
	os_interrupt_counter = 0;

	/* Clear the scheduling lock counter */
	os_lock_counter = 0;

	/* Clear the tasks counter */
	os_task_counter = 0;

	/* Clear the idle counter */
	os_idle_counter = 0;

	/* Inidicate the multiple tasks not started */
	os_is_running = FALSE;

	/* Clear the context switch counter */
	os_switch_counter = 0;

	/* Priority of current task */
	os_prio_current = 0;
	/* Priority of highest priority ready task */
	os_prio_high_ready = 0;

	/* Pointer to the highest priority ready TCB */
	os_tcb_ready_ptr = NULL;
	/* Pointer to the currently running TCB */
	os_tcb_current_ptr = NULL;
}

/**
 *
 */
static void
init_tcb_list(void)
{
	int i, j;
	struct os_tcb *tcb_ptr_current;
	struct os_tcb *tcb_ptr_next;

	/* Clear all the TCBs table and priority table */
	memset(os_tcb_table, 0, sizeof(os_tcb_table));
	memset(os_tcb_prio_table, 0, sizeof(os_tcb_prio_table));

#if ENABLE_DEBUG
	/* Display the os_tcb_table address */
	uart_print("\n[core] os_tcb_table address: ");
	uart_print_hex((unsigned int) os_tcb_table);

	/* Display the os_tcb_prio_table address */
	uart_print("\n[core] os_tcb_prio_table address: ");
	uart_print_hex((unsigned int) os_tcb_prio_table);
	uart_print("\n");
#endif

	/* Initialize all the TCBs table, link TCBs table to list */
	for (i = 0; i < NUM_TCB_TABLES - 1; ++i) {
		j = i + 1;
		tcb_ptr_current = &os_tcb_table[i];
		tcb_ptr_next = &os_tcb_table[j];

		tcb_ptr_current->tcb_next = tcb_ptr_next;
	}

	/* Set the last table of TCB point to NULL */
	tcb_ptr_current = &os_tcb_table[NUM_TCB_TABLES - 1];
	tcb_ptr_current->tcb_next = NULL;

	/* Initialize the TCBs list and the free TCBs list */
	os_tcb_list = NULL;
	os_tcb_list_tail = NULL;
	os_tcb_free_list = os_tcb_table;
}

/**
 *
 * @prio:
 * @stk_top:
 *
 * called by task.c/create_task()
 */
int
init_tcb(prio_t prio, stk_t *stk_ptr)
{
	/* For CPSR register */
	cpsr_t cpsr;
	struct os_tcb *tcb_ptr;

	/* Disable interrupt */
	enter_critical();

	/* Get a free TCB from the free TCBs list */
	tcb_ptr = os_tcb_free_list;

	if (tcb_ptr != NULL) {
		/* Update the free TCBs list pointer */
		os_tcb_free_list = os_tcb_free_list->tcb_next;

		exit_critical();

		/* Load stack pointer to TCB */
		tcb_ptr->tcb_stk_ptr = stk_ptr;
		/* Load task priority to TCB */
		tcb_ptr->tcb_prio = prio;
		/* Clear pend status */
		//tcb_ptr->tcb_pend = TASK_PEND_OK;
		/* Task is not delay */
		tcb_ptr->tcb_delay = 0;
		/* Task is not delete */
		tcb_ptr->tcb_del_req = FALSE;



		enter_critical();

		/* Set os_tcb_prio_table[prio] point to this TCB */
		os_tcb_prio_table[prio] = tcb_ptr;

		/* Link to TCBs chain */
		if (os_tcb_list_tail != NULL) {
			tcb_ptr->tcb_prev = os_tcb_list_tail;
			os_tcb_list_tail->tcb_next = tcb_ptr;
		}

		os_tcb_list_tail = tcb_ptr;

		if (os_tcb_list == NULL) {
			os_tcb_list = tcb_ptr;
			tcb_ptr->tcb_prev = NULL;
		}

		tcb_ptr->tcb_next = NULL;

		/* Task is ready to run */
		tcb_ptr->task_status = TASK_READY;

		/* Increate tasks counter */
		++os_task_counter;

#if ENABLE_DEBUG
		uart_print("Init tcb[");
		uart_print_int((int) tcb_ptr->tcb_prio);
		uart_print("] done\n");
#endif

		exit_critical();
		return 0;
	}

#if ENABLE_DEBUG
	uart_print("\n[Error] No more TCB for task.\n");
#endif

	errno = ERR_TASK_NO_MORE_TCB;
	exit_critical();
	return -1;
}

/**
 * sched() -
 *
 */
void
sched(void)
{
	/* Allocate storage for CPSR */
	cpsr_t cpsr;

	enter_critical();

	/* Schedule only if all ISRs done and scheduler is not locked */
	if ((os_interrupt_counter == 0) && (os_lock_counter == 0)) {
		sched_new();

		/* Point to the highest ready TCB */
		os_tcb_ready_ptr = os_tcb_prio_table[os_prio_high_ready];

		if (os_tcb_ready_ptr != os_tcb_current_ptr) {
			/* Increate the context switch counter */
			++os_switch_counter;

			/* Context switch */
			switch_task();
		}
		
	}

	exit_critical();
}

/**
 * Find the highest priority ready task in the os_tcb_prio_table
 */
static void
sched_new(void)
{
	struct os_tcb *tcb_ptr;
	int index;

	for (index = 0; index < NUM_TCB_TABLES; ++index) {
		if ((os_tcb_prio_table[index] != NULL) &&
		    (os_tcb_prio_table[index] != (struct os_tcb *) 1)) {
			tcb_ptr = os_tcb_prio_table[index];
			if (tcb_ptr->task_status == TASK_READY) {
				os_prio_high_ready = index;

#if ENABLE_DEBUG
				cpsr_t cpsr;
				enter_critical();
				uart_print("sched_new(): high prio = ");
				uart_print_int((int) os_prio_high_ready);
				uart_print("\n");
				exit_critical();
#endif

				return;
			}
		}
	}
}

/**
 * Start multiple task
 */
void
start_os(void)
{
	if (os_is_running == FALSE) {
		/* Find highest priority task */
		sched_new();

		os_prio_current = os_prio_high_ready;

		/* Point to highest priority task ready to run */
		os_tcb_ready_ptr = os_tcb_prio_table[os_prio_high_ready];
		os_tcb_current_ptr = os_tcb_ready_ptr;

		/* Start task */
		start_task();
	}
}


/**
 * increase os_interrupt_counter
 */
/*
void enter_interrupt(void)
{
	if (os_is_running) {
		if (os_interrupt_counter < 255)
			++os_interrupt_counter;
	}
}
*/

void exit_interrupt(void)
{
	//uart_print("exit_interrupt()\n");

	cpsr_t cpsr;

	if (os_is_running) {
		enter_critical();

		if (os_interrupt_counter > 0)
			--os_interrupt_counter;

		/* Reschedule only if all ISRs complete and not lock */
		if ((os_interrupt_counter == 0) && (os_lock_counter == 0)) {
			sched_new();
			/* Point to the highest ready TCB */
			os_tcb_ready_ptr =
					os_tcb_prio_table[os_prio_high_ready];

			if (os_prio_high_ready != os_prio_current) {
				/* Increate the context switch counter */
				++os_switch_counter;

				/* Interrupt level context switch */
				interrupt_switch_task();
			}
#if ENABLE_DEBUG
			uart_print("highe ready == current\n"
				"interrupt counter = ");
			uart_print_int((int) os_interrupt_counter);
			uart_print("\n");
#endif
		}
		exit_critical();
	}
}


void
time_tick(void)
{
	struct os_tcb *tcb_ptr;
	cpsr_t cpsr;

	enter_critical();
	/* Update the 32-bit time tick counter */
	++os_time;
	exit_critical();

	if (os_is_running == FALSE)
		return;

	/* Point at first TCB in TCBs list */
	tcb_ptr = os_tcb_list;
	
	/* Go through all TCBs in TCBs list */
	while (tcb_ptr != NULL) {
		enter_critical();
#if ENABLE_DEBUG
		uart_print("Go through tcb: ");
		uart_print_int((int) tcb_ptr->tcb_prio);
		uart_print("\n");
#endif
		if (tcb_ptr->tcb_delay != 0) {
			/* Decrement number of ticks to end of delay */
			--(tcb_ptr->tcb_delay);
#if ENABLE_DEBUG
			uart_print("now have ");
			uart_print_int(tcb_ptr->tcb_delay);
			uart_print(" ticks\n");
#endif
			if (tcb_ptr->tcb_delay == 0)
				change_task_status(tcb_ptr);
		}
		/* Point to next TCB */
		tcb_ptr = tcb_ptr->tcb_next;
		exit_critical();
	}
}

/*
 *
 */
static void
change_task_status(struct os_tcb *tcb_ptr)
{
#if ENABLE_DEBUG
	uart_print("prio ");
	uart_print_int((int) tcb_ptr->tcb_prio);
	uart_print(" change task status\n");
#endif


	/* If the task status is sleep, then clear it */
	if ((tcb_ptr->task_status & TASK_SLEEP) == TASK_SLEEP)
		tcb_ptr->task_status &= ~TASK_SLEEP;

/*
	// Clear not suspend task status
	if ((tcb_ptr->tcb_status & TASK_PEND_ANY) != TASK_READY) {
		tcb_ptr->tcb_status &= ~TASK_PEND_ANY;
		tcb_ptr->tcb_pend = TASK_PEND_OVER;
	} else {
		tcb_ptr->tcb_pend = TASK_PEND_OK;
	}

	// Make sure the suspend task don't be ready
	if ((tcb_ptr->tcb_status & TASK_SUSPEND) == TASK_READY)
		tcb_ptr->tcb_status = TASK_READY;
*/
}

/*
 *
 */
void
lock_schedule(void)
{
	cpsr_t cpsr;

	if (os_is_running) {
		enter_critical();

		/* Cannot call inside the ISR */
		if (os_interrupt_counter == 0) {
			if (os_lock_counter < 255)
				++os_lock_counter;
		}
		exit_critical();
	}
}

/*
 *
 */
void
unlock_schedule(void)
{
	cpsr_t cpsr;

	if (os_is_running) {
		enter_critical();
		if (os_lock_counter > 0) {
			--os_lock_counter;

			/*  */
			if (os_lock_counter == 0 && os_interrupt_counter == 0) {
				exit_critical();
				sched();
			}
		}
		exit_critical();
	}
}


static void
init_idle_task(void)
{
	create_task(idle_task, 0, &os_idle_task_stk[IDLE_TASK_STK_SIZE - 1],
			TASK_IDLE_PRIO);
}

void
idle_task(void *arg)
{
	cpsr_t cpsr;

	while (1) {
		enter_critical();
		//uart_print("os_idle_counter++\n");
		++os_idle_counter;
		exit_critical();
	}
}

static void
init_stat_task(void)
{
	create_task(stat_task, 0, &os_stat_task_stk[STAT_TASK_STK_SIZE - 1],
			TASK_STAT_PRIO);
	os_stat_task_ready = FALSE;
}

/*
 * call in the user application
 */
void
init_stat(void)
{
	cpsr_t cpsr;

	/* Delay 2 time-ticks */
	delay(2);

	enter_critical();
	/* Clear idle counter */
	os_idle_counter  = 0;
	exit_critical();

	/* delay 1 second */
	sleep(1);

	enter_critical();
	/* Get the idle counter */
	os_idle_counter_max = os_idle_counter;
	os_stat_task_ready = TRUE;
	exit_critical();
}

/*
 * Calculate the CPU usage.
 */
void
stat_task(void *arg)
{
	cpsr_t cpsr;

	/* Wait for stat task to ready */
	while (os_stat_task_ready == FALSE)
		sleep(1);

	while (1) {
		enter_critical();
		os_idle_counter_run = os_idle_counter;
		os_idle_counter = 0;
		exit_critical();
		os_cpu_usage =
			(u8) (100 * os_idle_counter_run / os_idle_counter_max);
		/* Wait for next second */
		sleep(1);
	}
	
}

void test(void)
{
	uart_print("restore task\n");
}
