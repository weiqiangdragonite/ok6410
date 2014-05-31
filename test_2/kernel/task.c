/*
 *
 */

#include "os.h"
#include "errno.h"
#include "uart.h"

static stk_t *
init_task_stack(void (*task)(void *arg), void *arg, stk_t *stk_top);


/**
 * create_task() -
 *
 */
int
create_task(void (*task)(void *arg), void *arg, stk_t *stk_top, prio_t prio)
{
	/* Task's stack pointer */
	stk_t *sp;
	/* Store CPSR status register value */
	cpsr_t cpsr;
	int result;

	if (OS_USR_TASKS > OS_MAX_USR_TASKS) {
#if ENABLE_DEBUG
		uart_print("\n[Error] User tasks must lower than 250\n");
#endif
		return -1;
	}

	/* Make sure the priority is in allowable range */
	if (prio > OS_LOWEST_PRIO) {
#if ENABLE_DEBUG
		uart_print("\n[Error] Task's priority is invalid\n");
#endif
		errno = ERR_PRIO_INVALID;
		return -1;
	}

	/* Disable interrupt */
	enter_critical();

	/* Make sure we don't create task in an ISR */
	if (os_interrupt_counter > 0) {
		/* Restore */
#if ENABLE_DEBUG
		uart_print("\n[Error] Cannot create task in the ISR\n");
#endif
		errno = ERR_TASK_CREATE_ISR;
		exit_critical();
		return -1;
	}

	/* Make sure task doesn't already exist at this priority */
	if (os_tcb_prio_table[prio] == NULL) {
		/* Mark this prio and prevent other to use */
		os_tcb_prio_table[prio] = (struct os_tcb *) 1;

#if ENABLE_DEBUG
		uart_print("[task] Create task, prio = ");
		uart_print_int((int) prio);
		uart_print("\n[task] Task's stack address: ");
		uart_print_hex((unsigned int) stk_top);
		uart_print("\n[task] Task address: ");
		uart_print_hex((unsigned int) task);
		uart_print("\n");
#endif

		exit_critical();

		/* Initialize the task's stack */
		sp = init_task_stack(task, arg, stk_top);

		/* Initlialize the TCB */
		result = init_tcb(prio, sp);

		if (result == -1) {
			enter_critical();
			/* Make this priority avaliable to others */
			os_tcb_prio_table[prio] = NULL;
			exit_critical();

			return -1;
		}

#if ENABLE_DEBUG
		enter_critical();
		uart_print("[task] Create prio ");
		uart_print_int((int) prio);
		uart_print(" task done\n");
		exit_critical();
#endif

		/* Start schedule if system is running */
		if (os_is_running == TRUE)
			sched();
		/* Else return */
		return 0;
	}
#if ENABLE_DEBUG
	uart_print("\n[Error] Task's priority already exist\n");
#endif
	errno = ERR_PRIO_EXIST;
	exit_critical();
	return -1;
}


/**
 *
 * @task: a pointer to the task code
 * @arg:  a pointer to the data area that passed to the task
 * @stk_top: a pointer to the top of the stack
 *
 * called by create_task()
 *
 * Return: Return the location of the new top-of-stacks pointer
 */
static stk_t *
init_task_stack(void (*task)(void *arg), void *arg, stk_t *stk_top)
{
	stk_t *stk_ptr;

	/* Load stack pointer */
	stk_ptr = stk_top;

	/* Task entery point (store in PC) */
	*stk_ptr = (stk_t) task;


	/* R14 (LR) */
	*(--stk_ptr) = (u32) 0x14141414;
	/* R12 */
	*(--stk_ptr) = (u32) 0x12121212;
	*(--stk_ptr) = (u32) 0x11111111;
	*(--stk_ptr) = (u32) 0x10101010;
	*(--stk_ptr) = (u32) 0x09090909;
	*(--stk_ptr) = (u32) 0x08080808;
	*(--stk_ptr) = (u32) 0x07070707;
	*(--stk_ptr) = (u32) 0x06060606;
	*(--stk_ptr) = (u32) 0x05050505;
	*(--stk_ptr) = (u32) 0x04040404;
	*(--stk_ptr) = (u32) 0x03030303;
	*(--stk_ptr) = (u32) 0x02020202;
	*(--stk_ptr) = (u32) 0x01010101;
	/* R0 store the pointer which point to argument */
	*(--stk_ptr) = (u32) arg;
	/* CPSR: ARM SYS mode enable IRQ and FIQ */
	*(--stk_ptr) = (u32) 0x0000001F;

	return stk_ptr;
}

/*
 *
 */
int
suspend_task(prio_t prio)
{
	/* suspend self */
	u8 self;
	cpsr_t cpsr;
	struct os_tcb *tcb_ptr;

	if (prio == TASK_IDLE_PRIO) {
		errno = ERR_TASK_SUSPEND_IDLE;
		return -1;
	}

	if (prio > OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {
		errno = ERR_PRIO_INVALID;
		return -1;
	}

	enter_critical();

	if (prio == OS_PRIO_SELF) {
		prio = os_tcb_current_ptr->tcb_prio;
		self = TRUE;
	} else if (prio == os_tcb_current_ptr->tcb_prio)
		self = TRUE;
	else
		self = FALSE;

	tcb_ptr = os_tcb_prio_table[prio];
	if (tcb_ptr == NULL) {
		errno = ERR_TASK_SUSPEND_PRIO;
		exit_critical();
		return -1;
	}

	/* Make task status to suspend */
	tcb_ptr->task_status |= TASK_SUSPEND;

#if ENABLE_DEBUG
	uart_print("[task] Task ");
	uart_print_int(tcb_ptr->tcb_prio);
	uart_print(" is suspend\n");
#endif

	exit_critical();

	if (self == TRUE)
		sched();

	return 0;
}

/*
 *
 */
int
resume_task(prio_t prio)
{
	cpsr_t cpsr;
	struct os_tcb *tcb_ptr;

	if (prio == TASK_IDLE_PRIO) {
		errno = ERR_TASK_SUSPEND_IDLE;
		return -1;
	}

	if (prio > OS_LOWEST_PRIO) {
		errno = ERR_PRIO_INVALID;
		return -1;
	}

	enter_critical();

	tcb_ptr = os_tcb_prio_table[prio];

	if (tcb_ptr == NULL) {
		errno = ERR_TASK_RESUME_PRIO;
		exit_critical();
		return -1;
	}

	//if ((tcb_ptr->task_status & TASK_SUSPEND) == TASK_READY) {
	//	errno = ERR_TASK_NOT_SUSPENDED;
	//	exit_critical();
	//	return -1;
	//}

	/* Remove suspend status */
	tcb_ptr->task_status &= ~TASK_SUSPEND;
	exit_critical();

#if ENABLE_DEBUG
	uart_print("[task] Task ");
	uart_print_int(tcb_ptr->tcb_prio);
	uart_print(" is resume\n");
#endif

	if (os_is_running)
		sched();

	return 0;
}

/*
 *
 */
int
change_task_prio(prio_t old_prio, prio_t new_prio)
{
	cpsr_t cpsr;
	

	/* task_idle_prio == os_lowest_prio */
	if (old_prio >= OS_LOWEST_PRIO && old_prio != OS_PRIO_SELF) {
		errno = ERR_PRIO_INVALID;
		uart_print("[task] Invalid task prio\n");
		return -1;
	}

	if (new_prio >= OS_LOWEST_PRIO) {
		errno = ERR_PRIO_INVALID;
		uart_print("[task] Invalid task prio\n");
		return -1;
	}

	enter_critical();

	if (os_tcb_prio_table[new_prio] != NULL) {
		errno = ERR_PRIO_EXIST;
		uart_print("[task] Task prio is exist\n");
		exit_critical();
		return -1;
	}

	if (old_prio == OS_PRIO_SELF)
		old_prio = os_tcb_current_ptr->tcb_prio;

	struct os_tcb *tcb_ptr;
	tcb_ptr = os_tcb_prio_table[old_prio];

	if (tcb_ptr == NULL) {
		errno = ERR_PRIO;
		exit_critical();
		return -1;
	}

	/* Remove TCB from old priority */
	os_tcb_prio_table[old_prio] = NULL;
	/* Change to new priority */
	os_tcb_prio_table[new_prio] = tcb_ptr;
	tcb_ptr->tcb_prio = new_prio;

	exit_critical();

	if (os_is_running)
		sched();

	return 0;
}

/*
 *
 */
int
delete_task(prio_t prio)
{
	cpsr_t cpsr;

	/* Cannot delete from ISR */
	if (os_interrupt_counter > 0) {
		errno = ERR_TASK_DEL_ISR;
		return -1;
	}

	if (prio == TASK_IDLE_PRIO) {
		errno = ERR_TASK_DEL_IDLE;
		return -1;
	}

	if (prio >= OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {
		errno = ERR_PRIO_INVALID;
		return -1;
	}

	enter_critical();

	if (prio == OS_PRIO_SELF)
		prio = os_tcb_current_ptr->tcb_prio;

	struct os_tcb *tcb_ptr;
	tcb_ptr = os_tcb_prio_table[prio];

	if (tcb_ptr == NULL) {
		errno = ERR_TASK_NOT_EXIST;
		exit_critical();
		return -1;
	}

	/* Make task to suspend */
	tcb_ptr->task_status = TASK_SUSPEND;

	/* Prevent time_tick() */
	tcb_ptr->tcb_delay = 0;
	//tcb_ptr->tcb_pend = TASK_PEND_OK;

	--os_task_counter;

	os_tcb_prio_table[prio] = NULL;

	/* Remove from TCBs chain */
	/* Last tcb */
	if (tcb_ptr->tcb_next == NULL) {
		os_tcb_list_tail = tcb_ptr->tcb_prev;
		tcb_ptr->tcb_prev->tcb_next = tcb_ptr->tcb_next;
	/* Middle tcb */
	} else if (tcb_ptr->tcb_prev != NULL) {
		tcb_ptr->tcb_next->tcb_prev = tcb_ptr->tcb_prev;
		tcb_ptr->tcb_prev->tcb_next = tcb_ptr->tcb_next;
	/* First tcb */
	} else {
		os_tcb_list = tcb_ptr->tcb_next;
		tcb_ptr->tcb_next->tcb_prev = NULL;
	}
	tcb_ptr->tcb_prev = NULL;
	tcb_ptr->tcb_next = os_tcb_free_list;
	os_tcb_free_list = tcb_ptr;

	exit_critical();

	if (os_is_running)
		sched();

	return 0;
}


/*
 *
 */
int
delete_task_request(prio_t prio)
{
	u8 status;
	cpsr_t cpsr;

	if (prio == TASK_IDLE_PRIO) {
		errno = ERR_TASK_DEL_IDLE;
		uart_print("[task] Cannot delete idle task\n");
		return -1;
	}

	if (prio >= OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {
		errno = ERR_PRIO_INVALID;
		uart_print("[task] Invalid task prio\n");
		return -1;
	}

	/**/
	if (prio == OS_PRIO_SELF || prio == os_tcb_current_ptr->tcb_prio) {
		enter_critical();
		status = os_tcb_current_ptr->tcb_del_req;
		exit_critical();
		return status;
	}

	enter_critical();

	struct os_tcb *tcb_ptr;
	tcb_ptr = os_tcb_prio_table[prio];

	/* Task not exist or task had already deleted */
	if (tcb_ptr == NULL) {
		errno = ERR_TASK_NOT_EXIST;
		exit_critical();
		return -1;
	}

	tcb_ptr->tcb_del_req = OS_TASK_DEL_REQ;

	exit_critical();
	return 0;
}
