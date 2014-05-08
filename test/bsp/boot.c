/*
 * ok6410/bsp/boot.c
 */

#include "s3c6410.h"
#include "uart.h"
#include "sdram.h"
#include "nand.h"
#include "timer.h"
#include "lcd.h"

static int update_program(void);


void
boot(void)
{
	char choose;

	/* Init uart */
	init_uart();

#if ENABLE_LCD
	/* Init LCD */
	config_lcd(480, 272, SCREEN_HOIZONTAL);
	init_lcd();
	//lcd_display_string(0, 0, COLOR_RED, COLOR_WHITE, "hello, world!");
	//lcd_clear_line(0, COLOR_WHITE);
#endif

	/* Menu */
	while (1) {
		uart_puts("\n");
		uart_puts("***** Bootloader Menu *****");
		uart_puts("[a] Update program");
		uart_puts("[b] Reboot");
		uart_puts("[c] Run OS");
		uart_puts("***************************");
		uart_print(">>> ");

		choose = uart_getc();
		uart_puts("\n");

		switch (choose) {
		case 'a':
			update_program();
			break;
		case 'b':
			uart_puts("Rebooting ...");
			reboot();
			/* Wait for Watch Dog Timer reset */
			while (1)
				continue;
		case 'c':
			/* Return and start run the os */
			return;
		default:
			break;
		}
	}
}


/*
 * call from boot() in SYS mode with disable FIQ and IRQ
 */
static int
update_program(void)
{
	/* We store new program at 0x5FC00000 */
	/* (about 4 MB to the end of memory) */
	unsigned int addr = 0x5FC00000;
	/* buffer is point to 0x5FC00000 */
	unsigned char *buffer = (unsigned char *) addr;
	unsigned int wait_time = 0;
	int start = 0;
	int length = 0;
	int size = 0;

	uart_puts("Please send binary file!");

#define USE_QT_COM

#ifdef USE_QT_COM
	size = uart_get_file_size();
	uart_print("file size = ");
	uart_print_int(size);
	uart_puts(" bytes\nReceiving binary file ...");
#endif

	/* Start to get file */
	while (1) {
		if (uart_get_file(&buffer[length])) {
		/* Has receive file */
		start = 1;
		wait_time = 0;
		++length;
		} else if (start) {
			++wait_time;
		}

#ifdef USE_QT_COM
		if (length == size)
			break;
#endif
		if (wait_time == 0x00FFFFFF)
			break;
	}

	if (length == 0) {
		uart_puts("No file receive.");
		return -1;
	}

	uart_print("\nReceived ");
	uart_print_int(length);
	uart_print(" bytes data.\n");

	/* Check for data */
	uart_print("\nThe first 16 byte are:");
	sdram_read(addr, addr + 0xF);
	uart_print("\nThe last 16 bytes are:");
	sdram_read(addr + length - 1 - 0xF, addr + length - 1);

	uart_puts("\nDo you sure update this program? (Y/n)");
	uart_print(" >> ");
	char ch = uart_getc();
	uart_puts("");

	if (ch != 'Y' && ch != 'y') {
		uart_puts("You have cancel update.");
		return -1;
	}

	uart_puts("Updating ...");

	/* Erase block 0 (128 pages, 128 * 4K = 512 K) */
	nand_erase_block(0);
	/* Write to bootloader */
	write_to_boot(addr, length);

	uart_puts("Update done!");

	return 0;
}


int
raise(int signum)
{
	uart_print("\nraise: Signal # ");
	uart_print_int(signum);
	uart_print(" caught.\n");

	return 0;
}
