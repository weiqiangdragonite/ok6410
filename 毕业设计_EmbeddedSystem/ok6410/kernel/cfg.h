/*
 *
 */

#ifndef CFG_H
#define CFG_H

/* User configure */


/* Defines the lowest priority that can be assigned (0 - 63) */
#define OS_LOWEST_PRIO		63

/* Max number of tasks in your application, MUST be >= 2 and <= 62 */
#define OS_MAX_TASKS		62

/* Max number of event */
#define OS_MAX_EVENTS		10

/* Enable statistics task */
#define ENABLE_STAT_TASK	0

/* Statistics task stack size */
#define STAT_TASK_STK_SIZE	128
/* Idle task stack size */
#define IDLE_TASK_STK_SIZE	128
/* Default task stack size */
#define STK_SIZE		512

/* Interrupt 100 times per second (1ms ~ 100ms) */
#define OS_TICKS_PER_SEC	100


/**/
#define OS_MAX_MEM_NUM		10


/* Debug infomation */
#define ENABLE_DEBUG		0


#endif	/* CFG_H */
