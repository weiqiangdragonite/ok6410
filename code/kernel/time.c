/*
 *
 */

#include "os.h"
#include "cfg.h"
#include "uart.h"
#include "errno.h"


/*
 *
 */
void
delay(unsigned int ticks)
{
	cpsr_t cpsr;

	/* See if trying to call from an ISR */
	if (os_interrupt_counter > 0)
		return;

	/* See if called with sceduler lock */
	if (os_lock_counter > 0)
		return;

	if (ticks == 0)
		return;

	enter_critical();

	/* Change the task status */
	os_tcb_current_ptr->tcb_status |= TASK_SLEEP;

	/* Delay ticks */
	os_tcb_current_ptr->tcb_delay = ticks;

	exit_critical();

	sched();
}


/*
 *
 */
void
msleep(unsigned int milisecond)
{
	/*
	 * precision = 1000 / OS_TICKS_PER_SEC
	 * ticks = milisecond / precision
	 */
	static unsigned int precision;
	unsigned int ticks;

	precision = 1000 / OS_TICKS_PER_SEC;

	if (milisecond < precision)
		return;

	/* Delay ticks */
	ticks = milisecond / precision;

#if ENABLE_DEBUG
	cpsr_t cpsr;
	enter_critical();
	uart_print("[time] sleep ");
	uart_print_int(ticks);
	uart_print(" ticks\n");
	exit_critical();
#endif

	delay(ticks);
}

/*
 *
 */
void
sleep(unsigned int second)
{
	msleep(second * 1000);
}

/*
 *
 */
void
sleep_hmsm(unsigned int hour, unsigned int minute,
	  unsigned int second, unsigned int milisecond)
{
	/* Compute the really ticks */
	unsigned int ms;

	ms = (hour * 3600 + minute * 60 + second) * 1000 + milisecond;
	msleep(ms);
}

/*
 *
 */
int
cancel_delay(prio_t prio)
{
	if (prio == TASK_IDLE_PRIO) {
		errno = ERR_TASK_DEL_IDLE;
		return -1;
	}

	if (prio >= OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {
		errno = ERR_PRIO_INVALID;
		return -1;
	}

	cpsr_t cpsr;
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

	if (tcb_ptr->tcb_delay != 0) {
		tcb_ptr->tcb_delay = 0;

		if ((tcb_ptr->tcb_status & TASK_SUSPEND) == TASK_READY)
			tcb_ptr->tcb_status = TASK_READY;
	}

	sched();

	return 0;
}

/*
 *
 */
u32
get_os_time(void)
{
	u32 ticks;
	cpsr_t cpsr;

	enter_critical();
	ticks = os_time;
	exit_critical();

	return ticks;
}

/*
 *
 */
void
set_os_time(u32 ticks)
{
	cpsr_t cpsr;

	enter_critical();
	os_time = ticks;
	exit_critical();
}
