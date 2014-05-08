
#include "os.h"
#include "timer.h"
#include "uart.h"
#include "led.h"
#include "time.h"
#include "memory.h"


stk_t main_stk[STK_SIZE];
stk_t second_stk[STK_SIZE];
stk_t third_stk[STK_SIZE];

stk_t task_stk[2][STK_SIZE];

void main_task(void *arg);
void second_task(void *arg);
void third_task(void *arg);

void fourth_task(void *arg);
void fifth_task(void *arg);

struct os_mem	*mem_buf;
u8		memory[NUM_BLOCK][BLOCK_LEN];

int time = 0;


int
main(void)
{
	init_os();

	/* Init sys time */
	init_sys_time();

	/* Init led */
	init_led();

	char *str = "hello ";

	/* This three tasks is for delete task */
	create_task(main_task, str, &main_stk[STK_SIZE - 1], 0);
	create_task(second_task, str, &second_stk[STK_SIZE - 1], 10);
	create_task(third_task, NULL, &third_stk[STK_SIZE - 1], 30);

	/* This task is for suspend task */
	create_task(fourth_task, str, &task_stk[0][STK_SIZE - 1], 40);

	/* This task is for memory */
	create_task(fifth_task, str, &task_stk[1][STK_SIZE - 1], 50);

	/* Create the memory buffer */
	mem_buf = create_mem(memory, NUM_BLOCK, BLOCK_LEN);

	start_os();

	return 0;
}

void
main_task(void *arg)
{
	while (1) {
		uart_print((char *) arg);
		uart_print("led 1\n");

		if (get_led_1() == LED_ON)
			set_led_1(LED_OFF);
		else
			set_led_1(LED_ON);

		if (time == 10) {
			uart_print("delete task 3\n");

			while (delete_task_request(30) != -1) {
				uart_print("waiting task 3 to delete\n");
				sleep(1);
			}
			uart_print("task 3 not exist now\n");
		}


		if (time == 20) {
			uart_print("[1] resume task 4\n");
			resume_task(40);
		}

		++time;

		/* Sleep 1 second */
		sleep(1);
	}
}

void
second_task(void *arg)
{
	while (1) {
		uart_print((char *) arg);
		uart_print("led 2\n");
		if (get_led_2() == LED_ON)
			set_led_2(LED_OFF);
		else
			set_led_2(LED_ON);

		/* Sleep 1 second */
		sleep(1);
	}
}

void
third_task(void *arg)
{
	while (1) {
		if (delete_task_request(30) == OS_TASK_DEL_REQ) {
			uart_print("[3]: I am going to delete myself\n");
			sleep(5);
			uart_print("[3]: I had deleted myself\n");
			delete_task(30);
		}

		uart_print("task three\n");
		sleep(1);
	}
}

void
fourth_task(void *arg)
{
	while (1) {
		if (time == 10) {
			uart_print("[4]: I am going to suspend myself\n");
			suspend_task(OS_PRIO_SELF);
		}

		uart_print((char *) arg);
		uart_print("task four\n");
		sleep(1);
	}
}

void
fifth_task(void *arg)
{
	int *mem_ptr;

	while (1) {
		uart_print("task five\n");

		if (time == 15) {
			uart_print("[5]: I am going to get 300 bytes memory\n");

			mem_ptr = (int *) get_mem(mem_buf, 300);

			*mem_ptr = 65536;
			uart_print("The data in the mem_ptr = ");
			uart_print_int(*mem_ptr);
			uart_print("\n");
		} else if (time == 25) {
			uart_print("[5]: I am going to release 300 bytes"
					"memory\n");

			free_mem(mem_buf, mem_ptr);
		}

		uart_print("free block = ");
		uart_print_int(mem_buf->num_free);
		uart_print("\n");
		sleep(1);
	}
}
