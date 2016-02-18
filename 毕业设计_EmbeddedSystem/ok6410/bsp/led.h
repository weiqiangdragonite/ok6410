/*
 *
 */


#ifndef LED_H
#define LED_H


/* led mode */
#define LED_ON  0
#define LED_OFF 1

/* function prototype */
void init_led(void);

void all_leds_on(void);
void all_leds_off(void);

void set_led_1(int mode);
void set_led_2(int mode);
void set_led_3(int mode);
void set_led_4(int mode);

int get_led_1(void);
int get_led_2(void);
int get_led_3(void);
int get_led_4(void);


#endif	/* LED_H */
