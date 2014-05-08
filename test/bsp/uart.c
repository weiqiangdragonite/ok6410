/******************************************************************************\
*
\******************************************************************************/

/*

\a  响铃(BEL)                                 7
\b  退格(BS) ，将当前位置移到前一列           8
\f  换页(FF)，将当前位置移到下页开头          12
\n  换行(LF) ，将当前位置移到下一行开头       10
\r  回车(CR) ，将当前位置移到本行开头         13
\t  水平制表(HT) （跳到下一个TAB位置）        9
\v  垂直制表(VT)                              11
\\  代表一个反斜线字符''\'
\'  代表一个单引号（撇号）字符
\'' 代表一个双引号字符
\0  空字符(NULL)                               0


换行
\r      Mac 
\n      Unix/Linux 
\r\n    Windows

*/

/*

s3c6410有4个uart

RXD0 -> GPA0
TXD0 -> GPA1

RXD1 -> GPA4
TXD1 -> GPA5

RXD2 -> GPB0
TXD2 -> GPB1

RXD3 -> GPB2
TXD3 -> GPB3

//RXD1 & TXD1 - GPA4 & GPA5 : [23:16]
GPACON = (GPACON & ~0xFF0000) | 0x22;
// RXD2 & TXD2 - GPB0 & GPB1 : [7:0]
GPBCON = (GPBCON & ~0xFF) | 0x22;
// RXD3 & TXD3 - GPB2 & GPB3 : [15:8]
GPBCON = (GPBCON & ~0xFF00) | 0x22

*/

#include "s3c6410.h"
#include "lcd.h"
#include "types.h"

#define UART_0  0
#define UART_1  1
#define UART_2  2
#define UART_3  3

extern void *lcd_memcpy(void *dest, const void *src, size_t n);


/*
 *
 */
void
init_uart(void)
{
	/*
	 * Default to use UART0:
	 * set GPA0 and GPA1 to UART;
	 * the OK6410 RXD0 is connect to GPA0;
	 * TXD0 is connect to GPA1;
	 * set GPACON[7:0] to 0010_0010
	 */
	GPACON = (GPACON & ~0xFF) | 0x22;

	/*
	 * Set the baud rate to 115200bps:
	 * DIV_VAL = (PCLK / (bps * 16)) - 1
	 *         = (66000000 / (115200 * 16)) - 1
	 *         = 34.8
	 * 13 / 16 = 0.8125
	 * UBRDIVn=34，UDIVSLOTn=0xDFDD
	 */
	UBRDIV0 = 34;
	UDIVSLOT0 = 0xDFDD;

	/*
	 * Set the ULCONn:
	 * Normal mode operation
	 * No parity
	 * One stop bit per frame
	 * 8-bit data
	 * 0000_0011
	 */
	ULCON0 = 3;

	/*
	 * Set the UCONn:
	 * clock is: PCLK
	 * Interrupt type is Level
	 * Disable time-out interrupt
	 * Do not generate error status interrupt
	 * Transmit/Receive mode is interrupt or polling
	 * 1011_0000_0101
	 */
	UCON0 = (UCON0 & ~0xFFF) | 0xB05;

	/* Set the UFCONn to disable FIFO */
	UFCON0 = 0;

	/* Set the UMCONn to disable flow control */
	UMCON0 = 0;

	/* Mask interrupt */
	UINTM0 = 0xF;
	UINTM1 = 0xF;
	UINTM2 = 0xF;
	UINTM3 = 0xF;

	//uart_putc('\n');
}

/*
void
set_uart_baud_rate(int uart, int baud_rate)
{
    int UBRDIVn;
    int UDIVSLOTn;
    
    // set user define baud rate
    float DIV_VAL = (float) 66000000 / (baud_rate * 16) - 1;
    UBRDIVn = (int) DIV_VAL;
    float point = DIV_VAL - UBRDIVn;
    
    int i = 0;
    while (1) {
        if ((i / 16.0) >= point)
            break;
        ++i;
    }
    
    switch (i) {
    case  0: UDIVSLOTn = 0x0000; break;
    case  1: UDIVSLOTn = 0x0080; break;
    case  2: UDIVSLOTn = 0x0808; break;
    case  3: UDIVSLOTn = 0x0888; break;
    case  4: UDIVSLOTn = 0x2222; break;
    case  5: UDIVSLOTn = 0x4924; break;
    case  6: UDIVSLOTn = 0x4A52; break;
    case  7: UDIVSLOTn = 0x54AA; break;
    case  8: UDIVSLOTn = 0x5555; break;
    case  9: UDIVSLOTn = 0xD555; break;
    case 10: UDIVSLOTn = 0xD5D5; break;
    case 11: UDIVSLOTn = 0xDDD5; break;
    case 12: UDIVSLOTn = 0xDDDD; break;
    case 13: UDIVSLOTn = 0xDFDD; break;
    case 14: UDIVSLOTn = 0xDFDF; break;
    case 15: UDIVSLOTn = 0xFFDF; break;
    case 16: UBRDIVn += 1; UDIVSLOTn = 0; break;
	}
    
    if (uart == UART_0) {
        UBRDIV0 = UBRDIVn;
        UDIVSLOT0 = UDIVSLOTn;
    } else if (uart == UART_1) {
        UBRDIV1 = UBRDIVn;
        UDIVSLOT1 = UDIVSLOTn;
    } else if (uart == UART_2) {
        UBRDIV2 = UBRDIVn;
        UDIVSLOT2 = UDIVSLOTn;
    } else if (uart == UART_3) {
        UBRDIV3 = UBRDIVn;
        UDIVSLOT3 = UDIVSLOTn;
    }
}
*/


/*
 * print char
 */
void
uart_putc(unsigned char ch)
{
	/* Wait for the transmit register empty */
	while (1) {
		if ((UTRSTAT0 & 6) == 6)
			break;
	}

	UTXH0 = ch;

#if ENABLE_LCD
	if (ch >= 128)
		return;

	/* 列 */
	unsigned int x = curr_col * FONT_WIDTH;
	/* 行 */
	unsigned int y = curr_row * FONT_HEIGHT;

	if (ch >= 32)
		lcd_display_char(x, y, COLOR_BLACK, COLOR_WHITE, ch);
	else if (ch == '\n') {
		/* 换行 */
		curr_col = -1;
		++curr_row;
	} else if (ch == 8) {
		/* backspace */
		--curr_col;
		return;
	} else
		return;

	/* 前进一列 */
	++curr_col;
	/* 列数为 0 - 59 */
	if (curr_col >= 60) {
		/* 换行 */
		curr_col = 0;
		++curr_row;
	}


	/* 行数为 0 - 16，超过16行，从第1行起往上移一行 */
	if (curr_row >= 17) {
		/* 1行总共有 480 * 16 * 4 = 30720 个字节，所以第1行的地址为
		   (FRAME_BUFFER + 480 * 16)
		   第1行到第16行总共有  480 * 16 * 16 * 4 = 491520 个字节 */

		/* (dest, src, size) */
		lcd_memcpy((int *) FRAME_BUFFER, (int *) FRAME_BUFFER + lcd_cfg.width * FONT_HEIGHT,
			480 * 16 * 16);

		/* 然后最后一行清零 */
		lcd_clear_line(16, COLOR_WHITE);

		--curr_row;
		curr_col = 0;
	}

#endif
}

/*
 * print string, with '\n'
 */
void
uart_puts(char *str)
{
	int i;
	for (i = 0; str[i] != '\0'; ++i) {
		uart_putc(str[i]);
	}

	uart_putc('\n');
}

/*
 * print string, no '\n'
 */
void
uart_print(char *str)
{
	int i;
	for (i = 0; str[i] != '\0'; ++i) {
		uart_putc(str[i]);
	}
}

/*
 * get char
 */
unsigned char
uart_getc(void)
{
    char buffer[80];
    int i = 0;
    
    while (1) {
        // get the char
        while (1) {
            if (UTRSTAT0 & 1) break;
        }
        buffer[i] = URXH0;
        
        // be careful, do not use  up/down/left/right key
        // backspace
        // 08 -> 退回前一个字符
        // 20 -> 输出空格（将前一个字符清空）
        // 08 -> 再退回去
        if (buffer[i] == 8 && i > 0) {
            uart_putc(buffer[i--]);
            buffer[i] = ' ';
            uart_putc(buffer[i]);
            uart_putc(8);
            continue;
        } else if (buffer[i] == 8 && i == 0) {
            continue;
        }
        
        // send to screen what you type
        if (i != 79) {
            uart_putc(buffer[i]);
        } else if (i == 79 && buffer[i] == 0xD) {
            uart_puts("");
            return buffer[0];
        }
        
        
        // 
        if (i == 0 && buffer[i] == 0xD) {
            uart_puts("");
            return '\0';
        }
        
        //
        if (i > 0 && buffer[i] == 0xD) {
            uart_puts("");
            return buffer[0];
        }
        
        // 
        if (++i == 80) i = 79;
    }
    
}




/*
 * 15 -> 0000000F'\0'
 * 16 -> 00000010'\0'
 */
static char *
int_to_hex(unsigned int data, char *num)
{
	int index;
	int remainder;

	index = 8;
	while (1) {
		remainder = data % 16;
		data /= 16;

		if (remainder >= 10 && remainder <= 15)
			num[--index] = remainder - 10 + 'A';
		else
			num[--index] = remainder + '0';

		if (data == 0)
			break;
	}
	while (index > 0)
		num[--index] = '0';

	/* null terminated */
	num[8] = '\0';

	return num;
}

/*
 * 0x00876543
 */
void
uart_print_hex(unsigned int data)
{
	char num[9];
	int i;

	int_to_hex(data, num);

	uart_putc('0');
	uart_putc('x');
	for (i = 0; num[i] != '\0'; ++i)
		uart_putc(num[i]);
}

/*
 * For little-endin
 * 0xEA000006 is store like this:
 *
 * high address +----+
 *              | EA |
 *              +----+
 *              | 00 |
 *              +----+
 *              | 00 |
 *              +----+
 *              | 06 |
 * low address  +----+
 *
 * So we get the address value EA000006, and print out: 060000EA
 * This function is used for update program and check for the binary file!
 */
void
uart_print_addr_value(unsigned int data)
{
	char num[9];

	int_to_hex(data, num);
	/* After int_to_hex(), we get num: EA000006 */

	/* EA_00_00_06 -> 06_00_00_EA */
	uart_putc(num[6]);
	uart_putc(num[7]);
	uart_putc(num[4]);
	uart_putc(num[5]);
	uart_putc(num[2]);
	uart_putc(num[3]);
	uart_putc(num[0]);
	uart_putc(num[1]);
}






void uart_gets(char *buffer)
{
    
    int i = 0;
    
    while (1) {
        // get the char
        while (1) {
            if (UTRSTAT0 & 1) break;
        }
        buffer[i] = URXH0;
        
        // be careful, do not use  up/down/left/right key
        // backspace
        if (buffer[i] == 8 && i > 0) {
            uart_putc(buffer[i--]);
            buffer[i] = ' ';
            uart_putc(buffer[i]);
            uart_putc(8);
            continue;
        } else if (buffer[i] == 8 && i == 0) {
            continue;
        }
        
        // send to screen what you type
        if (i != 79) {
            uart_putc(buffer[i]);
        } else if (i == 79 && buffer[i] == 0xD) {
            uart_puts("");
            buffer[i] = '\0';
            return;
        }
        
        
        // 
        if (i == 0 && buffer[i] == 0xD) {
            buffer[i] = '\0';
            uart_puts("");
            return;
        }
        
        //
        if (i > 0 && buffer[i] == 0xD) {
            uart_puts("");
            buffer[i] = '\0';
            return;
        }
        
        // 
        if (++i == 80) i = 79;
    }
    
}


void uart_scanf(char *type, int *addr)
{
    
}


int uart_get_int(void)
{
    char buffer[80];
    uart_gets(buffer);
    int num = 0;
    
    int i = 0;
    int sign = 1;
    if (buffer[0] == '-') {
        ++i;
        sign = -1;
    }

    for (; buffer[i] != '\0'; ++i) {
        if (!(buffer[i] >= '0' && buffer[i] <= '9')) return 0;
        num = num * 10 + buffer[i] - '0';
    }

    return sign * num;
}


/*
 *
 */
void
uart_print_int(int data)
{
	char num[80];
	int i;

	if (data < 0)
		uart_putc('-');

	i = 0;
	while (1) {
		num[i++] = data % 10 + '0';
		data /= 10;
		if (data == 0)
			break;
	}

	for (i -= 1; i >= 0; --i) {
		uart_putc(num[i]);
	}
}

/*
 *
 */
int
uart_get_file(unsigned char *buffer)
{
	if (UTRSTAT0 & 1) {
		*buffer = URXH0;
		return 1;
	}
	return 0;
}

/*
 *
 */
int
uart_get_file_size(void)
{
	char buffer[80];
	int num;
	int i;

	i= 0;
	while (i < 79) {
		/* Wait for have data */
		while ((UTRSTAT0 & 1) != 1)
			continue;

		buffer[i] = URXH0;

		/* Receive data end */
		if (buffer[i] == '\r' || buffer[i] == '\n')
			break;
		++i;
	}
	buffer[i] = '\0';

	num = 0;
	for (i = 0; buffer[i] != '\0'; ++i) {
		if (!(buffer[i] >= '0' && buffer[i] <= '9'))
			return 0;
		num = num * 10 + buffer[i] - '0';
	}

	return num;
}
