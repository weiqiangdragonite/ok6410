/*
 *
 */

#ifndef TIME_H
#define TIME_H

void delay(unsigned int ticks);

void msleep(unsigned int milisecond);

void sleep(unsigned int second);

void sleep_hmsm(unsigned int hour, unsigned int minute,
		unsigned int second, unsigned int milisecond);

#endif	/* TIME_H */
