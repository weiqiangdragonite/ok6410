#include "os.h"
#include "timer.h"
#include "uart.h"
#include "led.h"
#include "time.h"
#include "memory.h"
#include "lcd.h"




static unsigned int next = 1;

stk_t task_stk[20][STK_SIZE];

void main_task(void *arg);
void random_task(void *arg);

int rand(int range);
void srand(unsigned int seed);



int
main(void)
{
	init_os();

	/* Init sys time */
	init_sys_time();

	create_task(main_task, NULL, &task_stk[0][STK_SIZE - 1], 0);


	start_os();

	return 0;
}

void
main_task(void *arg)
{
	int index;
	int prio;
	for (index = 5, prio = 10; index < 15; ++index, ++prio)
		create_task(random_task, NULL, &task_stk[index][STK_SIZE - 1], prio);

	/* clear background */
	display_bg_color(COLOR_WHITE);

	init_stat();

	lcd_display_string(16, 0, COLOR_RED, COLOR_WHITE, "  CPU Usage:     %");
	lcd_display_string(16, 21, COLOR_RED, COLOR_WHITE, "Tasks: ");
	lcd_display_string(16, 35, COLOR_RED, COLOR_WHITE, "Task switch/sec: ");
	while (1) {
		lcd_display_string(16, 13, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(16, 13, COLOR_BLUE, COLOR_WHITE, os_cpu_usage);

		lcd_display_string(16, 28, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(16, 28, COLOR_BLUE, COLOR_WHITE, os_task_counter);

		lcd_display_string(16, 52, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(16, 52, COLOR_BLUE, COLOR_WHITE, os_switch_counter);

		os_switch_counter = 0;

		sleep(1);
	}
}

void
random_task(void *arg)
{
	unsigned int seed;
	int number;
	int column;
	int row;

	while (1) {
		seed = get_os_time();
		srand(seed);
		number = rand(10);
		column = rand(60);
		row = rand(15);
		lcd_display_int(row, column, COLOR_BLACK, COLOR_WHITE, number);

		msleep(500);
	}
}

/* 0 ~ range - 1 */
int
rand(int range)
{
	next = next * 1103515245 + 12345;
	return ((unsigned) (next / 65536) % range);
}

void
srand(unsigned int seed)
{
	next = seed;
}
