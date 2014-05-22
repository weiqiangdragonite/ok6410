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

int cancel_delay(prio_t prio);
u32 get_os_time(void);
void set_os_time(u32 ticks);

#endif	/* TIME_H */
