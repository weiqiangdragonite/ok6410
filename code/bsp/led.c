/*
 *
 */


#include "s3c6410.h"
#include "led.h"

void
init_led(void)
{
	/* led 1 ~ led 4 use GPM0 ~ GPM3 */
	/* Set the GPMCON[15:0]to be output - 0001 0001 0001 0001 */
	GPMCON &= ~0xFFFF;
	GPMCON |= 0x1111;

	/* Set the GPMPUD[7:0] to disable pull-up/down - 0000 0000 */
	GPMPUD &= ~0xFF;

	/* High level: turn off led; low level: turn on led */
	/* Set the GPMDAT[3:0] to high level(turn off all the led) - 1111 */
	GPMDAT |= 0xF;
}

void
all_leds_on(void)
{
	GPMDAT &= ~0xF;
}

void
all_leds_off(void)
{
	GPMDAT |= 0xF;
}

void
set_led_1(int mode)
{
	if (mode == LED_OFF)
		/* Turn off led 1 */
		GPMDAT |= 1;
	else if (mode == LED_ON)
		/* Turn on led 1 */
		GPMDAT &= ~1;
}

void
set_led_2(int mode)
{
	if (mode == LED_OFF)
		/* Turn off led 2 */
		GPMDAT |= 2;
	else if (mode == LED_ON)
		/* Turn on led 2 */
		GPMDAT &= ~2;
}

void
set_led_3(int mode)
{
	if (mode == LED_OFF)
		/* Turn off led 3 */
		GPMDAT |= 4;
	else if (mode == LED_ON)
		/* Turn on led 3 */
		GPMDAT &= ~4;
}

void
set_led_4(int mode)
{
	if (mode == LED_OFF)
		/* Turn off led 4 */
		GPMDAT |= 8;
	else if (mode == LED_ON)
		/* Turn on led 4 */
		GPMDAT &= ~8;
}


int
get_led_1(void)
{
	unsigned int mode = GPMDAT & 1;

	if (mode)
		return LED_OFF;
	else
		return LED_ON;
}

int
get_led_2(void)
{
	unsigned int mode = GPMDAT & 2;

	if (mode)
		return LED_OFF;
	else
		return LED_ON;
}

int
get_led_3(void)
{
	unsigned int mode = GPMDAT & 4;

	if (mode)
		return LED_OFF;
	else
		return LED_ON;
}

int
get_led_4(void)
{
	unsigned int mode = GPMDAT & 8;

	if (mode)
		return LED_OFF;
	else
		return LED_ON;
}
