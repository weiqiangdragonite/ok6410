
#include "os.h"
#include "timer.h"
#include "uart.h"
#include "led.h"
#include "time.h"


stk_t main_stk[STK_SIZE];
stk_t second_stk[STK_SIZE];
stk_t third_stk[STK_SIZE];

void main_task(void *arg);
void second_task(void *arg);
void third_task(void *arg);

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

	create_task(main_task, str, &main_stk[STK_SIZE - 1], 0);
	create_task(second_task, str, &second_stk[STK_SIZE - 1], 10);
	create_task(third_task, NULL, &third_stk[STK_SIZE - 1], 30);

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
		uart_print("3\n");
		sleep(1);
	}
}
