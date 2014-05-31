

#ifndef ERRNO_H
#define ERRNO_H


/*
 * Error status
 */



/* Error for task */
#define ERR_PRIO_INVALID	1
#define ERR_PRIO_EXIST		2
#define ERR_PRIO		3

#define ERR_TASK_CREATE_ISR	10
#define ERR_TASK_NO_MORE_TCB	11
#define ERR_TASK_SUSPEND_IDLE	12
#define ERR_TASK_SUSPEND_PRIO	13
#define ERR_TASK_RESUME_PRIO	14
#define ERR_TASK_NOT_SUSPENDED	15
#define ERR_TASK_DEL_ISR	16
#define ERR_TASK_DEL_IDLE	17
#define ERR_TASK_NOT_EXIST	18
#define ERR_TASK_DEL_REQ	19


#define ERR_EVENT_TYPE
#define ERR_EVENT_ISR
#define ERR_EVENT_LOCK

#define ERR_SEMAPHORE_ISR	30


#define ERR_MEM_INVALID_ADDR	40
#define ERR_MEM_INVALID_PART	41
#define ERR_MEM_INVALID_PTR	42
#define ERR_MEM_NO_FREE_BLOCK	43



u8	errno;




#endif	/* ERRNO_H */
