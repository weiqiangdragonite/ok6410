/*
 * ok6410/bsp/exceptions.c
 */

#include "s3c6410.h"
#include "uart.h"

void
handle_und(void)
{
	uart_print("\nundefined instruction\n");
	while (1)
		continue;
}

void
handle_swi(void)
{
	uart_print("\nswi\n");
	while (1)
		continue;
}

void
handle_pabt(void)
{
	uart_print("\nprefetch abort\n");
	while (1)
		continue;
}

void
handle_dabt(void)
{
	uart_print("\ndata abort\n");
	while (1)
		continue;
}

void
handle_irq(void)
{
	void (*the_isr)(void);

	if (VIC0IRQSTATUS) {
		the_isr = (void *) VIC0ADDRESS;
	} else if (VIC1IRQSTATUS) {
		the_isr = (void *) VIC1ADDRESS;
	}

	(*the_isr)();
}

void
handle_fiq(void)
{
	uart_print("\nfiq\n");
	while (1)
		continue;
}
