/*
 *
 */


#ifndef UART_H
#define UART_H


#define UART_0  0
#define UART_1  1
#define UART_2  2
#define UART_3  3




void init_uart(void);

void set_uart_baud_rate(int uart, int baud_rate);
void uart_0_isr(void);

void uart_putc(unsigned char ch);
void uart_puts(char *str);

unsigned char uart_getc(void);
void uart_gets(char *str);


void uart_print(char *str);
void uart_scanf(char *type, int *addr);


int uart_get_int(void);
void uart_print_int(int data);


void uart_print_hex(unsigned int data);
void uart_print_addr_value(unsigned int data);


int uart_get_file(unsigned char *buffer);
int uart_get_file_size(void);




#endif	/* UART_H */
