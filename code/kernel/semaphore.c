/*
 *
 */

#include "os.h"
#include "errno.h"

/*
 *
 */
struct os_event *
create_event_semaphore(u8 counter)
{
	cpse_t cpsr;
	struct os_event *event_ptr;

	if (os_interrupt_counter > 0) {
		errno = ERR_SEMAPHORE_ISR;
		return NULL;
	}

	enter_critical();
	event_ptr = os_event_free_list;
	if (os_event_free_list != NULL)
		os_event_free_list = os_event_free_list->event_ptr;

	exit_critical();

	if (event_ptr != NULL) {
		event_ptr->event_type = EVENT_SEMAPHORE;
		event_ptr->event_counter = counter;
		event_ptr->event_ptr = NULL;
		/* Initialize the  */
		init_wait_event_list(event_ptr);
	}

	return event_ptr;
}

/*
 *
 */
int
request_semaphore(struct os_event *event_ptr, u32 time_ticks)
{
	/* Validate the event type */
	if (event_ptr->event_type != EVENT_SEMAPHORE) {
		errno = ERR_EVENT_TYPE;
		return -1;
	}

	if (os_interrupt_counter > 0) {
		errno = ERR_EVENT_ISR;
		return -1;
	}

	if (os_lock_counter > 0) {
		errno = ERR_EVENT_LOCK;
		return -1;
	}

	enter_critical();
	/**/
	if (event_ptr->event_counter > 0) {
		--(event_ptr->event_counter);
		exit_critical();
		return 0;
	}

	/* Wait for event */
	os_tcb_current_ptr->tcb_status |= TASK_SEMAPHORE;
	os_tcb_current_ptr->tcb_pend = TASK_PEND_OK;
	os_tcb_current_ptr->tcb_delay = time_ticks;

	/* Suspend task until event occurs */
	wait_event_task(event_ptr);

	exit_critical();

	sched();

	/* Next highest priority task == current task */
	enter_critical();

	//switch (os_tcb_current_ptr->tcb_pend) {
	//}

	os_tcb_current_ptr->tcb_status = TASK_READY;
	os_tcb_current_ptr->tcb_pend = TASK_PEND_OVER;
	os_tcb_current_ptr->tcb_event_ptr = NULL;

	exit_critical();

	return 0;
}

int
post_semaphore(struct os_event *event_ptr)
{
	cpsr_t cpsr;

	if (event_ptr == NULL) {
		return -1;
	}

	if (event_ptr->event_type != EVENT_SEMAPHORE) {
		errno = ERR_EVENT_TYPE;
		return -1;
	}

	if (event_ptr->event_prio_group != 0) {
		event_task_ready(event_ptr, TASK_SEMAPHORE, TASK_PEND_OK);
		exit_critical();
		sched();
		return 0;
	}

	if (event_ptr->event-counter < 255) {
		++(event_ptr->event_counter);
		exit_critical();
		return 0;
	}

	exit_critical();
	return 0;
}
