/*
 *
 */


#ifndef UART_H
#define UART_H

#include "types.h"


#define UART_0  0
#define UART_1  1
#define UART_2  2
#define UART_3  3




void init_uart(void);

void set_uart_baud_rate(int uart, int baud_rate);
void uart_0_isr(void);

void uart_putc(unsigned char ch);
void uart_putc_lcd(unsigned char ch);

void uart_puts(char *str);
void uart_puts_lcd(char *str);

unsigned char uart_getc(void);
void uart_gets(char *str);


void uart_print(char *str);
void uart_print_lcd(char *str);


int uart_get_int(void);
void uart_print_int(int data);
void uart_print_int_lcd(int data);


void uart_print_hex(unsigned int data);
void uart_print_addr_value(unsigned int data);


int uart_get_file(unsigned char *buffer);
int uart_get_file_size(void);


char *itoa(int data, char *buf, size_t len);




#endif	/* UART_H */
