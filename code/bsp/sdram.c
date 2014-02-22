/*
 *
 */

#include "s3c6410.h"
#include "uart.h"


/*
 * This function init the SDRAM.
 */
void
init_sdram(void)
{
	/* Set DRAM controller sequence*/

	/* 1. Set P1MEMCCMD to Configure */
	P1MEMCCMD = 4;

	/* 2. */
	/* Set the refresh period */
	P1REFRESH = 1037;

	/* CAS Latency */
	P1CASLAT = 6;

	/* DQSS */
	P1T_DQSS = 1;

	/* MRD */
	P1T_MRD = 2;

	/* RAS */
	P1T_RAS = 7;

	/* RC */
	P1T_RC = 10;

	/* RCD */
	P1T_RCD = (1 << 3) | 4;

	/* RFC */
	P1T_RFC = (8 << 5) | 11;

	/* RP */
	P1T_RP = (1 << 3) | 4;

	/* RRD */
	P1T_RRD = 3;

	/* WR */
	P1T_WR = 3;

	/* WTR */
	P1T_WTR = 2;

	/* XP */
	P1T_XP = 1;

	/* XSR */
	P1T_XSR = 17;

	/* ESR */
	P1T_ESR = 17;

	/* 3. Set the P1MEMCFG register */
	/* set the [5:0] to 011010 */
	P1MEMCFG = (P1MEMCFG & 0xFFFFFFC0) | 0x1A;

	/* 4. Set the P1MEMCFG2 register */
	/* set the [12:0] to 0_1011_0100_0101 */
	P1MEMCFG2 = 0xB45;

	/* 5. Set the P1_chip_0_cfg register */
	/* Set the [16] to Bank-Row-Column organization */
	P1CHIP0CFG |= (1 << 16);

	/** Set the SDRAM sequence */

	/* 6. Set P1DIRECTCMD: */
	/* to NOP */
	P1DIRECTCMD = 0xC0000;
	/* to PrechargeAll */
	P1DIRECTCMD = 0;
	/* to Autorefresh */
	P1DIRECTCMD = 0x40000;
	/* to Autorefresh */
	P1DIRECTCMD = 0x40000;
	/* to MRS */
	P1DIRECTCMD = 0x80032;
	/* to EMRS */
	P1DIRECTCMD = 0xA0000;

	/* 7. Set MEM_SYS_CFG[5:0] to NFCON */
	MEM_SYS_CFG = 0;

	/* 8. Set P1MEMCCMD to Go */
	P1MEMCCMD = 0;

	/* 9. Wait P1MEMSTAT to become Ready */
	while (1) {
		if ((P1MEMSTAT & 0x3) == 1)
			break;
	}
}



/*
 * @start: address like 0x50000000
 * @end: address like 0x50000010
 */
void
sdram_read(unsigned int start, unsigned int end)
{
	/* We don't change the value which point to */
	const unsigned int *start_ptr;
	int column;

	start_ptr = (unsigned int *) start;
	column = 0;
	while (1) {
		if (start >= end)
			break;

		if (column == 0 || column == 4) {
			uart_print("\n");
			/* Print the address */
			uart_print_hex(start);
			uart_print(" :  ");
			column = 0;
		}

		/* Print the value */
		uart_print_addr_value(*start_ptr++);
		uart_print("  ");
		++column;
		start += 4;
	}
	uart_print("\n");
}
