

#include "os.h"
#include "timer.h"
#include "uart.h"
#include "led.h"
#include "time.h"
#include "memory.h"
#include "lcd.h"


u8 memory[NUM_BLOCK][BLOCK_LEN];
stk_t task_stk[250][STK_SIZE];

struct os_mem *mem_buffer;

u8 need_create_task;
u8 need_delete_task;
u8 user_prio = 99;
u8 mem_prio = 49;
u8 need_mem_task;
u8 release_mem_task;
prio_t old_prio, new_prio;


void main_task(void *arg);
void create_user_task(void *arg);
void delete_user_task(void *arg);
void user_task(void *arg);
void sus_task(void *arg);
void res_task(void *arg);
void prio_task(void *arg);
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

	/* Create memory */
	mem_buffer = create_mem(memory, NUM_BLOCK, BLOCK_LEN);	

	/* Create and delete other tasks */
	create_task(create_user_task, NULL, &task_stk[5][STK_SIZE - 1], 5);
	create_task(delete_user_task, NULL, &task_stk[6][STK_SIZE - 1], 6);

	/* This two task used for suspend and resume task */
	create_task(sus_task, NULL, &task_stk[10][STK_SIZE - 1], 10);
	create_task(res_task, NULL, &task_stk[11][STK_SIZE - 1], 11);

	/* This task will change prio */
	create_task(prio_task, NULL, &task_stk[30][STK_SIZE - 1], 30);

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
		lcd_display_int(13, 16, COLOR_BLUE, COLOR_WHITE, mem_buffer->num_blocks);

		lcd_display_string(13, 35, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(13, 35, COLOR_BLUE, COLOR_WHITE, mem_buffer->num_free);

		lcd_display_string(13, 55, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(13, 55, COLOR_BLUE, COLOR_WHITE, mem_buffer->num_blocks - mem_buffer->num_free);

		/* System */
		lcd_display_string(16, 7, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(16, 7, COLOR_BLUE, COLOR_WHITE, os_cpu_usage);

		lcd_display_string(16, 31, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(16, 31, COLOR_BLUE, COLOR_WHITE, os_switch_counter);

		lcd_display_string(16, 49, COLOR_WHITE, COLOR_WHITE, "          ");
		lcd_display_int(16, 49, COLOR_BLUE, COLOR_WHITE, get_os_time());

		os_switch_counter = 0;

		msleep(1000);
	}
}


void
create_user_task(void *arg)
{
	need_create_task = 0;

	while (1) {
		if (need_create_task) {
			++user_prio;
			create_task(user_task, &user_prio, &task_stk[user_prio][STK_SIZE - 1], user_prio);
			need_create_task = 0;
		}

		sleep(1);
	}
}

void
delete_user_task(void *arg)
{
	need_delete_task = 0;
	u8 curr_prio;
	while (1) {
		curr_prio = user_prio;
		if (curr_prio <= 99 && curr_prio > 250) {
			need_delete_task = 0;
			sleep(1);
			continue;
		}

		if (need_delete_task) {

			while (delete_task_request(curr_prio) != -1) {
				uart_print_lcd("[app] I am task 6, I am going to delete task ");
				uart_print_int_lcd(curr_prio);
				uart_print_lcd("\n");

				sleep(2);
			}
			need_delete_task = 0;
		}

		sleep(1);
	}
}

void
user_task(void *arg)
{
	u8 prio = *(u8 *) arg;
	while (1) {
		uart_print_lcd("[app] I am task ");
		uart_print_int_lcd(prio);


		if (delete_task_request(OS_PRIO_SELF) == OS_TASK_DEL_REQ) {
			uart_print_lcd(", I am going to delete myself\n");
			--user_prio;
			delete_task(OS_PRIO_SELF);
		}
		uart_print_lcd("\n");

		sleep(2);
	}
}


void
sus_task(void *arg)
{
	while (1) {
		uart_print_lcd("[app] I am task 10, I am waiting to suspend\n");

		sleep(1);
	}
}

void
res_task(void *arg)
{
	int time = 0;

	while (1) {
		++time;
		if (time == 10) {
			uart_print_lcd("[app] I am task 11, I am going to suspend task 10\n");
			suspend_task(10);
		}

		if (time == 20) {
			uart_print_lcd("[app] I am task 11, I am going to resume task 10\n");
			resume_task(10);
			time = 0;
		}

		sleep(1);
	}
}


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


void
get_mem_task_1(void *arg)
{
	int time = 0;
	u32 *ptr;
	while (1) {
		++time;
		if (time == 5) {
			uart_print_lcd("[app] I am task ");
			uart_print_int_lcd(os_tcb_current_ptr->tcb_prio);
			uart_print_lcd(", I am going to get one memory\n");

			ptr = get_mem(mem_buffer, mem_buffer->block_len);
			*ptr = get_os_time();
			uart_print_lcd("[app] Memory content is: ");
			uart_print_int_lcd(*ptr);
			uart_print_lcd("\n");
		}

		if (time == 20) {
			uart_print_lcd("[app] I am task ");
			uart_print_int_lcd(os_tcb_current_ptr->tcb_prio);
			uart_print_lcd(", I am going to release one memory\n");

			free_mem(mem_buffer, ptr);
			time = 0;
		}

		sleep(1);
	}
}

void
get_mem_task_2(void *arg)
{
	int time = 0;
	u32 *ptr;
	while (1) {
		++time;
		if (time == 10) {
			uart_print_lcd("[app] I am task ");
			uart_print_int_lcd(os_tcb_current_ptr->tcb_prio);
			uart_print_lcd(", I am going to get one memory\n");

			ptr = get_mem(mem_buffer, mem_buffer->block_len);
			*ptr = get_os_time();
			uart_print_lcd("[app] Memory content is: ");
			uart_print_int_lcd(*ptr);
			uart_print_lcd("\n");
		}

		if (time == 30) {
			uart_print_lcd("[app] I am task ");
			uart_print_int_lcd(os_tcb_current_ptr->tcb_prio);
			uart_print_lcd(", I am going to release one memory\n");

			free_mem(mem_buffer, ptr);
			time = 0;
		}

		sleep(1);
	}
}

void
create_mem_task(void *arg)
{
	need_mem_task = 0;
	

	while (1) {
		if (need_mem_task) {
			++mem_prio;
			create_task(get_mem_task, &mem_prio, &task_stk[mem_prio][STK_SIZE - 1], mem_prio);
			need_mem_task = 0;
		}

		sleep(1);
	}
}

void
delete_mem_task(void *arg)
{
	release_mem_task = 0;
	u8 curr_prio;

	while (1) {
		curr_prio = mem_prio;
		if (curr_prio <= 49 && curr_prio >= 100) {
			release_mem_task = 0;
			sleep(1);
			continue;
		}

		if (release_mem_task) {

			while (delete_task_request(curr_prio) != -1) {
				//uart_print_lcd("[app] I am task 6, I am going to delete task ");
				//uart_print_int_lcd(curr_prio);
				//uart_print_lcd("\n");

				sleep(2);
			}
			release_mem_task = 0;
		}

		sleep(1);
	}
}

void
get_mem_task(void *arg)
{
	u8 prio = *(u8 *) arg;

	uart_print_lcd("[app] I am task ");
	uart_print_int_lcd(prio);
	uart_print_lcd(", I am going to get one memory\n");

	u8 *ptr = get_mem(mem_buffer, mem_buffer->block_len);
	if (ptr == NULL)
		uart_print_lcd("[app] Cannot get memory\n");

	while (1) {
		uart_print_lcd("[app] I am task ");
		uart_print_int_lcd(prio);
		uart_print_lcd(", I am using one memory\n");

		if (delete_task_request(OS_PRIO_SELF) == OS_TASK_DEL_REQ) {
			uart_print_lcd("[app] I am task ");
			uart_print_int_lcd(prio);
			uart_print_lcd(", I am going to delete myself\n");
			free_mem(mem_buffer, ptr);
			--mem_prio;
			delete_task(OS_PRIO_SELF);
		}

		sleep(2);
	}
}
