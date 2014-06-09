

#include "os.h"
#include "timer.h"
#include "uart.h"
#include "led.h"
#include "time.h"
#include "memory.h"
#include "lcd.h"



stk_t task_stk[OS_USR_TASKS][STK_SIZE];


u8 need_create_task;
u8 need_delete_task;
u8 user_prio = 99;
u8 mem_prio = 49;
u8 need_mem_task;
u8 release_mem_task;
//prio_t old_prio, new_prio;


void main_task(void *arg);
void create_user_task(void *arg);
void delete_user_task(void *arg);
void user_task(void *arg);
void sus_task(void *arg);
void res_task(void *arg);
//void prio_task(void *arg);
void get_mem_task_1(void *arg);
void get_mem_task_2(void *arg);
void create_mem_task(void *arg);
void delete_mem_task(void *arg);
void get_mem_task(void *arg);


int
main(void)
{
	init_os();

	create_task(main_task, NULL, &task_stk[0][STK_SIZE - 1], 0);

	uart_print("\nStart running tasks\n\n");
	start_os();

	return 0;
}


void
main_task(void *arg)
{

	/* clear background */
	display_bg_color(COLOR_WHITE);

	/* Init sys time */
	init_sys_time();

	/**/
	init_stat();
	

	/* Create and delete other tasks */
	create_task(create_user_task, NULL, &task_stk[5][STK_SIZE - 1], 5);
	create_task(delete_user_task, NULL, &task_stk[6][STK_SIZE - 1], 6);

	/* This two task used for suspend and resume task */
	create_task(sus_task, NULL, &task_stk[10][STK_SIZE - 1], 10);
	create_task(res_task, NULL, &task_stk[11][STK_SIZE - 1], 11);

	/* This task will change prio */
	//create_task(prio_task, NULL, &task_stk[30][STK_SIZE - 1], 30);

	/* This task will get and free memory */
	create_task(create_mem_task, NULL, &task_stk[15][STK_SIZE - 1], 15);
	create_task(delete_mem_task, NULL, &task_stk[16][STK_SIZE - 1], 16);
	create_task(get_mem_task_1, NULL, &task_stk[17][STK_SIZE - 1], 17);
	create_task(get_mem_task_2, NULL, &task_stk[18][STK_SIZE - 1], 18);




	lcd_display_string(9,  0, COLOR_RED, COLOR_WHITE, "-------------------  Task   Information  -------------------");
	lcd_display_string(10, 2, COLOR_RED, COLOR_WHITE, "App tasks: ");
	lcd_display_string(10, 19, COLOR_RED, COLOR_WHITE, "Total tasks: ");
	lcd_display_string(10, 38, COLOR_RED, COLOR_WHITE, "Current task: ");

	lcd_display_string(12, 0, COLOR_RED, COLOR_WHITE, "-------------------  Memory Information  -------------------");
	lcd_display_string(13, 2, COLOR_RED, COLOR_WHITE, "Total memory: ");
	lcd_display_string(13, 22, COLOR_RED, COLOR_WHITE, "Free memory: ");
	lcd_display_string(13, 42, COLOR_RED, COLOR_WHITE, "Used memory: ");


	lcd_display_string(15, 0, COLOR_RED, COLOR_WHITE, "-------------------  System Information  -------------------");
	lcd_display_string(16, 2, COLOR_RED, COLOR_WHITE, "CPU:    %");
	lcd_display_string(16, 14, COLOR_RED, COLOR_WHITE, "Task switch/sec: ");
	lcd_display_string(16, 36, COLOR_RED, COLOR_WHITE, "System time: ");

	while (1) {
		//uart_print_lcd("[app] Task 0 is refresh system info\n");


		/* Task */
		lcd_display_string(10, 13, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(10, 13, COLOR_BLUE, COLOR_WHITE, os_task_counter - 2);

		lcd_display_string(10, 32, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(10, 32, COLOR_BLUE, COLOR_WHITE, os_task_counter);

		/* Memory */
		lcd_display_string(13, 16, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(13, 16, COLOR_BLUE, COLOR_WHITE, os_mem_ptr->num_blocks);

		lcd_display_string(13, 35, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(13, 35, COLOR_BLUE, COLOR_WHITE, os_mem_ptr->num_free);

		lcd_display_string(13, 55, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(13, 55, COLOR_BLUE, COLOR_WHITE, os_mem_ptr->num_blocks - os_mem_ptr->num_free);

		/* System */
		lcd_display_string(16, 7, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(16, 7, COLOR_BLUE, COLOR_WHITE, os_cpu_usage);

		lcd_display_string(16, 31, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(16, 31, COLOR_BLUE, COLOR_WHITE, os_switch_counter);

		lcd_display_string(16, 49, COLOR_WHITE, COLOR_WHITE, "          ");
		lcd_display_int(16, 49, COLOR_BLUE, COLOR_WHITE, get_os_time());

		os_switch_counter = 0;


		uart_print("\n[status] Task: ");
		uart_print_int(os_task_counter);
		uart_print(", Free memory: ");
		uart_print_int(os_mem_ptr->num_free);
		uart_print(", Used memory: ");
		uart_print_int(os_mem_ptr->num_blocks - os_mem_ptr->num_free);
		uart_print("\n");

		msleep(1000);
	}
}


/* task 5 */
void
create_user_task(void *arg)
{
	need_create_task = 0;

	while (1) {
		if (need_create_task) {
			++user_prio;
			//if (user_prio > OS_USR_TASKS)
			create_task(user_task, &user_prio, &task_stk[user_prio][STK_SIZE - 1], user_prio);
			need_create_task = 0;
		}

		sleep(1);
	}
}

/* task 6 */
void
delete_user_task(void *arg)
{
	need_delete_task = 0;
	//u8 curr_prio;

	while (1) {
		//curr_prio = user_prio;

		if (need_delete_task) {

			if (user_prio <= 99) {
				need_delete_task = 0;
				continue;
			}

			while (delete_task_request(user_prio) != -1) {
				uart_print("-- Task 6, going to delete task ");
				uart_print_int(user_prio);
				uart_print("\n");

				sleep(2);
			}
			need_delete_task = 0;
		}

		sleep(1);
	}
}

/* task 100 ~ */
void
user_task(void *arg)
{
	u8 prio = *(u8 *) arg;
	while (1) {
		uart_print_lcd("-- Task ");
		uart_print_int_lcd(prio);


		if (delete_task_request(OS_PRIO_SELF) == OS_TASK_DEL_REQ) {
			uart_print_lcd(", going to delete myself\n");
			--user_prio;
			delete_task(OS_PRIO_SELF);
		}
		uart_print_lcd("\n");

		sleep(2);
	}
}


/* task 10 */
void
sus_task(void *arg)
{
	while (1) {
		uart_print_lcd("-- Task 10, waiting to suspend\n");

		sleep(1);
	}
}

/* task 11 */
void
res_task(void *arg)
{
	int time = 0;

	while (1) {
		++time;
		if (time == 5) {
			uart_print_lcd("-- Task 11, going to suspend task 10\n");
			suspend_task(10);
		}

		if (time == 10) {
			uart_print_lcd("-- Task 11, going to resume task 10\n");
			resume_task(10);
			time = 0;
		}

		sleep(1);
	}
}


/*
void
prio_task(void *arg)
{
	old_prio = os_tcb_current_ptr->tcb_prio;
	new_prio = old_prio;
	int res;

	while (1) {
		old_prio = os_tcb_current_ptr->tcb_prio;

		uart_print_lcd("[app] I am task ");
		uart_print_int_lcd(old_prio);

		if (old_prio != new_prio && new_prio >= 0 && new_prio < TASK_STAT_PRIO) {
			uart_print_lcd(", I will change to task ");
			uart_print_int_lcd(new_prio);
			uart_print_lcd("\n");

			res = change_task_prio(old_prio, new_prio);
			if (res == -1) {
				uart_print_lcd("[app] I am task ");
				uart_print_int_lcd(old_prio);
				uart_print_lcd(", I cannot change to task ");
				uart_print_int_lcd(new_prio);
			} else {
				uart_print_lcd("[app] I am task ");
				uart_print_int_lcd(new_prio);
				uart_print_lcd(", my old task prio is ");
				uart_print_int_lcd(old_prio);
			}
		}
		uart_print_lcd("\n");

		sleep(1);
	}
}
*/


/* task 17 */
void
get_mem_task_1(void *arg)
{
	int time = 0;
	u32 *ptr = NULL;

	while (1) {
		++time;
		if (time == 5) {
			//uart_print_lcd("Task 17,");
			//uart_print_int_lcd(os_tcb_current_ptr->tcb_prio);
			uart_print_lcd("-- Task 17, going to get one memory\n");

			ptr = get_mem(BLOCK_LEN);
			if (ptr == NULL) {
				uart_print_lcd("[error] Task 17 cannot get one memory\n");
				time = 0;
				sleep(1);
				continue;
			}

			*ptr = get_os_time();
			uart_print_lcd("-- Task 17, memory content is: ");
			uart_print_int_lcd(*ptr);
			uart_print_lcd("\n");
		}

		if (time == 20) {
			//uart_print_lcd("Task ");
			//uart_print_int_lcd(os_tcb_current_ptr->tcb_prio);
			uart_print_lcd("-- Task 17, going to release one memory\n");

			free_mem(ptr);
			ptr = NULL;
			time = 0;
		}

		if (ptr != NULL)
			uart_print_lcd("-- Task 17, using one memory\n");

		sleep(1);
	}
}

/* task 18 */
void
get_mem_task_2(void *arg)
{
	int time = 0;
	u32 *ptr = NULL;

	while (1) {
		++time;
		if (time == 10) {
			//uart_print_lcd("[app] I am task ");
			//uart_print_int_lcd(os_tcb_current_ptr->tcb_prio);
			uart_print_lcd("-- Task 18, going to get one memory\n");

			ptr = get_mem(BLOCK_LEN);
			if (ptr == NULL) {
				uart_print_lcd("[error] Task 18 cannot get one memory\n");
				time = 0;
				sleep(1);
				continue;
			}

			*ptr = get_os_time();
			uart_print_lcd("-- Task 18, memory content is: ");
			uart_print_int_lcd(*ptr);
			uart_print_lcd("\n");
		}

		if (time == 30) {
			//uart_print_lcd("[app] I am task ");
			//uart_print_int_lcd(os_tcb_current_ptr->tcb_prio);
			uart_print_lcd("-- Task 18, going to release one memory\n");

			free_mem(ptr);
			ptr = NULL;
			time = 0;
		}

		if (ptr != NULL)
			uart_print_lcd("-- Task 18, using one memory\n");

		sleep(1);
	}
}

/* task 15 */
void
create_mem_task(void *arg)
{
	need_mem_task = 0;
	

	while (1) {
		if (need_mem_task) {
			++mem_prio;
			if (mem_prio >= 100) {
				uart_print_lcd("[error] Cannot create more memory task\n");
				need_mem_task = 0;
				continue;
			}
			create_task(get_mem_task, &mem_prio, &task_stk[mem_prio][STK_SIZE - 1], mem_prio);
			need_mem_task = 0;
		}

		sleep(1);
	}
}

/* task 16 */
void
delete_mem_task(void *arg)
{
	release_mem_task = 0;
	//u8 curr_prio;

	while (1) {
		//curr_prio = mem_prio;
		if (mem_prio <= 49) {
			release_mem_task = 0;
			sleep(1);
			continue;
		}

		if (release_mem_task) {

			while (delete_task_request(mem_prio) != -1) {
				uart_print("-- Task 16, going to delete task ");
				uart_print_int(mem_prio);
				uart_print("\n");

				sleep(2);
			}
			release_mem_task = 0;
		}

		sleep(1);
	}
}

/* task 50 ~ 99 */
void
get_mem_task(void *arg)
{
	u8 prio = *(u8 *) arg;

	uart_print_lcd("-- Task ");
	uart_print_int_lcd(prio);
	uart_print_lcd(" going to get one memory\n");

	u8 *ptr = NULL;

	while (1) {
		if (ptr == NULL) {
			ptr = get_mem(BLOCK_LEN);

			if (ptr == NULL) {
				uart_print_lcd("[error] Task ");
				uart_print_int_lcd(prio);
				uart_print_lcd(" cannot get one memory\n");

				sleep(2);
				continue;
			}
		}

		uart_print_lcd("-- Task ");
		uart_print_int_lcd(prio);
		uart_print_lcd(", using one memory\n");

		if (delete_task_request(OS_PRIO_SELF) == OS_TASK_DEL_REQ) {
			uart_print_lcd("-- Task ");
			uart_print_int_lcd(prio);
			uart_print_lcd(", going to delete myself\n");
			free_mem(ptr);
			--mem_prio;
			delete_task(OS_PRIO_SELF);
		}

		sleep(2);
	}
}
