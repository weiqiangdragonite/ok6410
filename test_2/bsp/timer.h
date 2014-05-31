/*
 *
 */


#ifndef TIMER_H
#define TIMER_H


void init_pwm(void);

void init_sys_time(void);
void timer_2_isr(void);

/*
void init_timer_1(void);
void start_timer_1(void);
void stop_timer_1(void);
void timer_1_isr(void);
*/

// rtc

typedef struct {
    int second;
    int minute;
    int hour;
    int day;
    int date;
    int month;
    int year;
} CALENDAR;

CALENDAR get_rtc_time(void);

void read_rtc_time(void);
void bcd_to_ascii(char value);
int get_rtc_second(void);
int get_rtc_minute(void);
int get_rtc_hour(void);
int get_rtc_date(void);
int get_rtc_day(void);
int get_rtc_month(void);
int get_rtc_year(void);
void change_rtc_time(void);
void change_rtc_year(int value);
void change_rtc_month(int value);
void change_rtc_date(int value);
void change_rtc_day(int value);
void change_rtc_hour(int value);
void change_rtc_minute(int value);
void change_rtc_second(int value);


// wdt
void reboot(void);


#endif	/* TIMER_H */
