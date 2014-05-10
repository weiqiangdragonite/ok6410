/*


*/

#include "os.h"
#include "timer.h"
#include "uart.h"
#include "led.h"
#include "time.h"
#include "memory.h"
#include "lcd.h"



#define N	100

int green_time = 60;
int red_time = 60;


//u32 memory[];

u32 buffer[N];
u8 empty = N;
u8 full = 0;

int produce = 1;
int consume = 1;



void producer_task(void *arg);
void consumer_task(void *arg);
void status_task(void *arg);


stk_t task_stk[4][STK_SIZE];


int
main(void)
{
	init_os();

	/* Init sys time */
	init_sys_time();

	create_task(producer_task, NULL, &task_stk[0][STK_SIZE - 1], 10);
	create_task(consumer_task, NULL, &task_stk[1][STK_SIZE - 1], 20);

	create_task(status_task, NULL, &task_stk[2][STK_SIZE - 1], 5);


	/* Create the memory buffer */
	//mem_buf = create_mem(memory, NUM_BLOCK, BLOCK_LEN);

	start_os();

	return 0;
}

void
producer_task(void *arg)
{
	int in = 0;
	int i;
	while (1) {
		if (empty == 0) {
			sleep(1);
			continue;
		}

		for (i = 0; i < produce; ++i) {
			buffer[in] = full;
			in = (in + 1) % N;

			--empty;
			++full;

			//uart_puts("Producer produce one product.");

			if (empty == 0) {
				sleep(1);
				continue;
			}
		}


		//sleep(1);
	}
}

void
consumer_task(void *arg)
{
	int out = 0;
	int i;
	while (1) {
		if (full == 0) {
			sleep(1);
			continue;
		}

		for (i = 0; i < consume; ++i) {
			buffer[out] = 0;
			out = (out + 1) % N;

			--full;
			++empty;

			//uart_puts("    Consumer consume one product.");

			if (full == 0) {
				sleep(1);
				continue;
			}
		}

		//sleep(1);
	}
}


void
status_task(void *arg)
{
	while (1) {
		uart_print("\nEmpty buffer = ");
		uart_print_int(empty);

		msleep(500);
	}
}
