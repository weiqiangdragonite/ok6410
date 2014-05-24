/*
 *
 */

#ifndef CFG_H
#define CFG_H

/* User configure */

/* Max number of tasks in your application, and the lowest user task's
   priority is (OS_USR_TASKS - 1), the system idle and statistics task's
   priority is OS_USR_TASKS and (OS_USR_TASKS + 1)
   Because system tasks + user tasks <= 255, so max user tasks is 254 */
#define OS_USR_TASKS		250


/* Enable statistics task */
#define ENABLE_STAT_TASK	1



/* Interrupt 100 times per second (1ms ~ 100ms) */
#define OS_TICKS_PER_SEC	100





/* Default statistics task stack size: (128 * 4) bytes */
#define STAT_TASK_STK_SIZE	128
/* Default idle task stack size */
#define IDLE_TASK_STK_SIZE	128
/* Default task stack size */
#define STK_SIZE		128




/* Memory */
#define OS_MAX_MEM_NUM		10

#define NUM_BLOCK		32
#define BLOCK_LEN		32


/* Debug infomation */
#define ENABLE_DEBUG		1




#endif	/* CFG_H */
