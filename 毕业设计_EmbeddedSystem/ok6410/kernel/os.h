/*
 * kernel/os.h
 *
 * This file defines the system global variables.
 */

#ifndef OS_H
#define OS_H

#include "types.h"
#include "cfg.h"


/*
 * Define Macro
 */

/* Need this? */
#ifndef NULL
#define NULL		((void *) 0)
#endif

#ifndef TRUE
#define TRUE		1
#endif

#ifndef FALSE
#define FALSE		0
#endif

/**/
#define enter_critical()	{cpsr = save_cpsr();}
#define exit_critical()		{restore_cpsr(cpsr);}


/* Size of the ready table */
#define OS_READY_TABLE_SIZE	((OS_LOWEST_PRIO) / 8 + 1)

/* Size of event table */
#define OS_EVENT_TABLE_SIZE	((OS_LOWEST_PRIO) / 8 + 1)

/* Number of system tasks */
#define OS_SYS_TASKS		2

/* Number of TCB tables */
#define NUM_TCB_TABLES		((OS_MAX_TASKS) + (OS_SYS_TASKS))

/* Task delete request */
#define OS_TASK_DEL_REQ		0xFF

/*
 * Task status
 */

/* Task not ready: 0xFF */
/* Task is ready to run */
#define TASK_READY		0x00
/* Task is sleep */
#define TASK_SLEEP		0x01
/* Task is suspend */
#define TASK_SUSPEND		0x02
/* Pending on semaphore */
#define TASK_SEMAPHORE		0x04
/* Pending on mailbox */
#define TASK_MBOX		0x08
/* Pending on queue */
#define TASK_QUEUE		0x10
/* Pending on mutex */
#define TASK_MUTEX		0x20


#define TASK_PEND_ANY		(TASK_SEMAPHORE | TASK_MBOX | TASK_QUEUE | \
				 TASK_MUTEX | TASK_SLEEP)

/* Pending status OK, not pending, or oending complete */
#define TASK_PEND_OK		0
/* Pending time out */
#define TASK_PEND_OVER		1
/* Pending aborted */
#define TASK_PEND_ABORT		2


/* Event types */
#define	EVENT_SEMAPHORE		1
#define EVENT_MUTEX		2


/* Statistic task priority */
#define TASK_STAT_PRIO		(OS_LOWEST_PRIO - 1)
/* IDLE task priority */
#define TASK_IDLE_PRIO		(OS_LOWEST_PRIO)


/* Self prio */
#define OS_PRIO_SELF		0xFF



/*
 *
 */
struct os_mem {
	void	*mem_addr;
	void	*mem_free_list;
	u32	block_len;
	u32	num_blocks;
	u32	num_free;
};


/*
 *
 */
struct os_event {
	u8	event_type;
	u8	event_counter;
	void	*event_ptr;
	prio_t	event_prio_table[OS_EVENT_TABLE_SIZE];
	prio_t	event_prio_group;
};


/*
 * struct os_tcb - task control block
 * @
 *
 */
struct os_tcb {
	stk_t		*tcb_stk_ptr;

	struct os_tcb	*tcb_next;
	struct os_tcb	*tcb_prev;

	prio_t		tcb_prio;
	u8		tcb_status;
	u8		tcb_pend;
	u32		tcb_delay;

	u8		tcb_del_req;

	u8		tcb_x;
	u8		tcb_y;
	prio_t		tcb_bit_x;
	prio_t		tcb_bit_y;

	struct os_event *tcb_event_ptr;
};




/*
 * Define global variable
 */

/* Current value of system time (in ticks) */
volatile u32	os_time;

/* Interrupt nesting counter */
extern u8	os_interrupt_counter;
/* Flag indicating that kernel is running or not */
extern u8	os_is_running;
/* Priority of current task */
extern prio_t	os_prio_current;
/* Priority of highest priority ready task */
extern prio_t	os_prio_high_ready;

/* Scheduling lock counter */
u8	os_lock_counter;
/* Tasks create counter */
u8	os_task_counter;
/* Counter of number of context switches */
u32	os_switch_counter;
/* Idle task counter  */
volatile u32	os_idle_counter;
/* Max value that idle task counter can get in one second */
u32	os_idle_counter_max;


/* Idle task stack */
stk_t	os_idle_task_stk[IDLE_TASK_STK_SIZE];
/* Statistic task stack */
stk_t	os_stat_task_stk[STAT_TASK_STK_SIZE];



/* The priority TCBs table (store pointer which point to TCB)  */
struct os_tcb	*os_tcb_prio_table[OS_LOWEST_PRIO + 1];
/* The TCBs struct table */
struct os_tcb	os_tcb_table[NUM_TCB_TABLES];


/* Pointer to current running TCB */
extern volatile struct os_tcb	*os_tcb_current_ptr;
/* Pointer to highest priority ready TCB */
extern volatile struct os_tcb	*os_tcb_ready_ptr;

/* The TCBs double linked list */
struct os_tcb	*os_tcb_list;
/* Point to the last tcb of os_tcb_list */
struct os_tcb	*os_tcb_list_tail;
/* Point to the list of free TCBs */
struct os_tcb	*os_tcb_free_list;


/**/
struct os_event	os_event_table[OS_MAX_EVENTS];

struct os_event	*os_event_free_list;


struct os_mem	*os_mem_free_list;
struct os_mem	os_mem_table[OS_MAX_MEM_NUM];


/*
 * Function prototypes
 */

extern cpsr_t save_cpsr(void);
extern void restore_cpsr(cpsr_t cpsr);
extern void start_task(void);
extern void switch_task(void);

extern void *memset(void *s, int c, size_t n);

extern void init_os(void);

extern int create_task(void (*task)(void *arg), void *arg,
			stk_t *stk_top, prio_t prio);

extern int init_tcb(prio_t prio, stk_t *stk_ptr);

extern void sched(void);

extern void start_os(void);

extern void time_tick(void);

extern void interrupt_switch_task(void);

void lock_schedule(void);
void unlock_schedule(void);

void idle_task(void *arg);
void stat_task(void *arg);


int suspend_task(prio_t prio);
int resume_task(prio_t prio);
int delete_task(prio_t prio);
int delete_task_request(prio_t prio);



void test(void);


#endif	/* OS_H */
