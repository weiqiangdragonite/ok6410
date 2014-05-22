/*
 * kernel/os.h
 *
 * This file defines the system global variables.
 */

#ifndef OS_H
#define OS_H

#include "types.h"
#include "cfg.h"




/* Save and restore CPSR macro */
#define enter_critical()	{ cpsr = save_cpsr(); }
#define exit_critical()		{ restore_cpsr(cpsr); }


/* Number of system tasks */
#define OS_SYS_TASKS		2

/* Defines the lowest priority that can be assigned */
#define OS_LOWEST_PRIO		((OS_USR_TASKS) + (OS_SYS_TASKS) - 1)

/* Number of TCB tables */
#define NUM_TCB_TABLES		((OS_USR_TASKS) + (OS_SYS_TASKS))

/* Statistic task priority */
#define TASK_STAT_PRIO		((OS_LOWEST_PRIO) - 1)
/* IDLE task priority */
#define TASK_IDLE_PRIO		(OS_LOWEST_PRIO)


/* Task self priority */
#define OS_PRIO_SELF		0xFF

/* Task delete request */
#define OS_TASK_DEL_REQ		0xFF






/*
 * Task status
 */
/* Task is ready to run */
#define TASK_READY		0x00
/* Task is sleep */
#define TASK_SLEEP		0x01
/* Task is suspend */
#define TASK_SUSPEND		0x02


/*
 * struct os_tcb - task control block
 * @
 *
 */
struct os_tcb {
	stk_t		*tcb_stk_ptr;
	prio_t		tcb_prio;

	struct os_tcb	*tcb_next;
	struct os_tcb	*tcb_prev;

	u8		task_status;
	u32		tcb_delay;

	u8		tcb_del_req;
};

/*
 * struct os_mem
 */
struct os_mem {
	void	*mem_addr;
	void	*mem_free_list;
	u32	block_len;
	u32	num_blocks;
	u32	num_free;
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

/* Idle task counter */
volatile u32	os_idle_counter;
/* Max value that idle task counter can get in one second */
u32	os_idle_counter_max;
/* Value reached by idle counter at run time in 1 one second  */
u32	os_idle_counter_run;         
/* Statistic task is ready */
u8	os_stat_task_ready;
/* CPU usage */
u8	os_cpu_usage;


/* Idle task stack */
stk_t	os_idle_task_stk[IDLE_TASK_STK_SIZE];
/* Statistic task stack */
stk_t	os_stat_task_stk[STAT_TASK_STK_SIZE];



/* The priority TCBs table (store pointer which point to TCB) */
struct os_tcb	*os_tcb_prio_table[NUM_TCB_TABLES];
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
void init_stat(void);
void stat_task(void *arg);


int suspend_task(prio_t prio);
int resume_task(prio_t prio);
int delete_task(prio_t prio);
int delete_task_request(prio_t prio);

extern void init_mem_list(void);



void test(void);


#endif	/* OS_H */
