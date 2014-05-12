/*

#define EAST_WEST	1
#define WEST_EAST	2
#define SOUTH_NORTH	3
#define NORTH_SOUTH	4

// 
green_led_signal = 1;
yellow_led_status = 0;

south_north_status = 0;
east_west_status = 0;

// 绿灯和红灯默认时间为60秒，可通过按键中断来增加或减少时间
green_time = 60;
red_time = green_time;

north_south_task()
{
	while (1) {
		//
		if (yellow_led_status) {
			// 延时2个节拍
			delay(2);
			continue;
		}

		// 申请绿灯信号量
		if (green_led_signal == 1) {
			--green_led_signal;

			// 南北向显示绿灯
			lcd_display();

		} else if (green_led_signal == 0) {
			// 南北向显示红灯
			lcd_display();

			// 休眠红灯显示时间
			south_north_status = 0;
			sleep(red_time);
			continue;
		}

		south_north_status = 1;
		// 休眠绿灯显示时间，其中减3秒的时间为黄灯时间
		sleep(green_time - 3);

		// 显示3秒黄灯
		yellow_led_status = 1;
		lcd_display();
		sleep(3);

		// 释放绿灯信号量
		++green_led_signal;
		yellow_led_status = 0;

		// 休眠4个节拍，让东西方向申请绿灯信号量
		delay(4);
	}
}


east_west_task()
{
	while (1) {
		//
		if (yellow_led_status) {
			// 延时2个节拍
			delay(2);
			continue;
		}

		// 申请绿灯信号量
		if (green_led_signal == 1) {
			--green_led_signal;

			// 东西向显示绿灯
			lcd_display();

		} else if (green_led_signal == 0) {
			// 东西向显示红灯
			lcd_display();

			// 休眠红灯显示时间
			east_west_status = 0;
			sleep(red_time);
			continue;
		}

		east_west_status = 1;
		// 休眠绿灯显示时间，其中减3秒的时间为黄灯时间
		sleep(green_time - 3);

		// 显示3秒黄灯
		yellow_led_status = 1;
		lcd_display();
		sleep(3);

		// 释放绿灯信号量
		++green_led_signal;
		yellow_led_status = 0;

		// 休眠4个节拍，让南北方向申请绿灯信号量
		delay(4);
	}
}

people_east_west_task()
{
	int flag = 0;
	int old_flag = 1;

	while (1) {
		if (east_west_status) {
			// 东西向通行，人行禁止
			flag = 0;
		} else {
			// 东西向禁止，人行道通行
			flag = 1;
		}

		if (old_flag != flag) {
			lcd_display_people_line(SOUTH_NORTH, flag);
			old_flag = flag;
		}

		sleep(1);
	}
}

people_north_south_task()
{
	int flag = 0;
	int old_flag = 1;

	while (1) {
		if (south_north_status) {
			// 南北向通行，人行禁止
			flag = 0;
		} else {
			// 南北向禁止，人行道通行
			flag = 1;
		}

		if (old_flag != flag) {
			lcd_display_people_line(WEST_EAST, flag);
			old_flag = flag;
		}

		sleep(1);
	}
}


*/


#include "os.h"
#include "timer.h"
#include "uart.h"
#include "led.h"
#include "time.h"
#include "memory.h"
#include "lcd.h"


#define EAST_WEST	1
#define WEST_EAST	2
#define SOUTH_NORTH	3
#define NORTH_SOUTH	4



u8 green_led_signal = 1;
u8 yellow_led_status = 0;

u8 south_north_status = 0;
u8 east_west_status = 0;

/* 绿灯和红灯默认时间为60秒，可通过按键中断来增加或减少时间 */
int green_time = 60;
int red_time = 60;
int yellow_time = 3;



void north_south_task(void *arg);
void east_west_task(void *arg);
void people_east_west_task(void *arg);
void people_north_south_task(void *arg);
void display_time_task(void *arg);


stk_t task_stk[4][STK_SIZE];


int
main(void)
{
	init_os();

	/* Init sys time */
	init_sys_time();


	create_task(north_south_task, NULL, &task_stk[0][STK_SIZE - 1], 5);
	create_task(east_west_task, NULL, &task_stk[1][STK_SIZE - 1], 6);

	/*  */
	//create_task(people_north_south_task, NULL, &task_stk[3][STK_SIZE - 1], 10);
	//create_task(people_east_west_task, NULL, &task_stk[2][STK_SIZE - 1], 11;

	create_task(display_time_task, NULL, &task_stk[2][STK_SIZE - 1], 10);


	/**/
	lcd_create_traffic_bg();
	/* 全部先显示红灯 */
	lcd_display_south_north_light(0);
	lcd_display_east_west_light(0);
	lcd_display_people_line(SOUTH_NORTH, 0);
	lcd_display_people_line(EAST_WEST, 0);

	/* 右下角显示红绿灯时间 */
	lcd_display_int(16, 55, COLOR_BLUE, COLOR_WHITE, green_time);


	start_os();

	return 0;
}

void
north_south_task(void *arg)
{
	while (1) {
		if (yellow_led_status) {
			/* 延时2个节拍 */
			delay(2);
			continue;
		}

		/* 申请绿灯信号量 */
		if (green_led_signal == 1) {
			--green_led_signal;

			/* 南北向人行道显示红灯 */
			lcd_display_people_line(SOUTH_NORTH, 0);

			/* 南北向显示绿灯 */
			lcd_display_south_north_light(1);

		} else if (green_led_signal == 0) {
			/* 南北向显示红灯 */
			lcd_display_south_north_light(0);

			/* 南北向人行道显示绿灯 */
			lcd_display_people_line(SOUTH_NORTH, 1);

			south_north_status = 0;
			/* 休眠红灯显示时间 */
			sleep(red_time);
			continue;
		}

		south_north_status = 1;
		/* 休眠绿灯显示时间 */
		sleep(green_time - yellow_time);

		/* 显示3秒黄灯 */
		yellow_led_status = 1;
		lcd_display_south_north_light(2);
		sleep(yellow_time);

		/* 释放绿灯信号量 */
		++green_led_signal;
		yellow_led_status = 0;

		/* 休眠4个节拍，让东西方向申请绿灯信号量 */
		delay(4);
	}
}


void
east_west_task(void *arg)
{
	while (1) {
		if (yellow_led_status) {
			/* 延时2个节拍 */
			delay(2);
			continue;
		}

		/* 申请绿灯信号量 */
		if (green_led_signal == 1) {
			--green_led_signal;

			/* 东西向人行道显示红灯 */
			lcd_display_people_line(EAST_WEST, 0);

			/* 东西向显示绿灯 */
			lcd_display_east_west_light(1);

		} else if (green_led_signal == 0) {
			/* 东西向显示红灯 */
			lcd_display_east_west_light(0);

			/* 东西向人行道显示绿灯 */
			lcd_display_people_line(EAST_WEST, 1);

			east_west_status = 0;
			/* 休眠红灯显示时间 */
			sleep(red_time);
			continue;
		}

		east_west_status = 1;
		/* 休眠绿灯显示时间 */
		sleep(green_time - yellow_time);

		/* 显示3秒黄灯 */
		yellow_led_status = 1;
		lcd_display_east_west_light(2);
		sleep(3);

		/* 释放绿灯信号量 */
		++green_led_signal;
		yellow_led_status = 0;

		/* 休眠4个节拍，让南北方向申请绿灯信号量 */
		delay(4);
	}
}


/*
 * 上下的人行道
 * 一开始，南北向通行，人行道继续禁止
 */
void
people_north_south_task(void *arg)
{
	int flag = 0;
	int old_flag = 1;

	while (1) {
		if (south_north_status) {
			/* 南北向通行，人行道禁止 */
			flag = 0;
		} else {
			/* 南北向禁止，人行通行 */
			flag = 1;
		}

		if (old_flag != flag) {
			lcd_display_people_line(SOUTH_NORTH, flag);
			old_flag = flag;
		}

		sleep(1);
	}
}

/*
 * 左右的人行道
 * 一开始，东西向禁行，所以人行道通行
 */
void
people_east_west_task(void *arg)
{
	int flag = 0;
	int old_flag = 0;

	while (1) {
		if (east_west_status) {
			/* 东西向通行，人行禁止 */
			flag = 0;
		} else {
			/* 东西向禁止，人行道通行 */
			flag = 1;
		}

		if (old_flag != flag) {
			lcd_display_people_line(EAST_WEST, flag);
			old_flag = flag;
		}

		sleep(1);
	}
}

void display_time_task(void *arg)
{
	int time = 0;
	while (1) {
		if (time == 0)
			time = green_time;

		/* 左下角显示 */
		lcd_display_string(16, 5, COLOR_WHITE, COLOR_WHITE, "   ");
		lcd_display_int(16, 5, COLOR_BLUE, COLOR_WHITE, time);
		sleep(1);
		--time;
	}
}

