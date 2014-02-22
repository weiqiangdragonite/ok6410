/*
 * ok6410/bsp/clock.c
 *
 * The init system clock is set to:
 *	APLL - 533 MHz, MPLL - 266 MHz, ARMCLK  - 532 MHz, HCLKX2   - 266 MHz
 *	HCLK - 133 MHz, PCLK - 66  MHz, CLKJPEG -  66 MHz, CLKSECUR -  66 MHz
 *
 */

/*

6410 has three PLLs which are APLL for ARM operating clock, MPLL for main
operating clock, and EPLL for special purpose. The operating clocks are divided
into three groups. The first thing is ARM clock, which is generated from APLL.
MPLL generates the main system clocks, which are used for operating AXI, AHB,
and APB bus operation. The last group is generated from EPLL. Mainly, the
generated clocks are used for peripheral IPs, i.e., UART, IIS, IIC, and etc.

Typical value setting for clock dividers
APLL    MPLL    ARMCLK    HCLKX2    HCLK    PCLK    CLKJPEG    CLKSECUR
266     266     0/266     0/266     1/133   3/66    3/66       3/66
400     266     0/400     0/266     1/133   3/66    3/66       3/66
533     266     0/533     0/266     1/133   3/66    3/66       3/66
667     266     0/667     0/266     1/133   3/66    3/66       3/66

APLL/MPLL: ENABLE[31] MDIV[25:16] PDIV[13:8] SDIV[2:0]
Fout = MDIV * Fin / (PDIV * (2 ^ SDIV))
     = 266  * 12  / (3    * (2 ^ 2)) = 266 MHz
     = 400  * 12  / (3    * (2 ^ 2)) = 400 MHz
     = 266  * 12  / (3    * (2 ^ 1)) = 532 MHz
     = 333  * 12  / (3    * (2 ^ 1)) = 666 MHz
       
EPLL_CON0: ENABLE[31] MDIV[23:16] PDIV[13:8] SDIV[2:0]
EPLL_CON1: KDIV[15:0]
Fout = (MDIV + KDIV / (2 ^ 16)) * Fin / (PDIV * (2 ^ SDIV))

*/


#include "s3c6410.h"

/*
 * This function set the CPU clock(APLL and MPLL).
 * mode 1: 266/266
 *      2: 400/266
 *      3: 533/266
 *      4: 667/226
 */
void
set_clock(int mode)
{
	/* 1. Set the PLL lock time */
	/* The xPLL lock time is set to default 0x0000FFFF */
	APLL_LOCK = 0x0000FFFF;
	MPLL_LOCK = 0x0000FFFF;
	EPLL_LOCK = 0x0000FFFF;

	/* 2. Set the others control registers */
	/*
	 * Set SYNMODE to Asynchronous mode
	 * (When the ARMCLK != HCLK, we need to set this!)
	 * and set the SYNCMUXSEL to use DOUTMPLL.
	 * OTHERS &= 0xFFFFFF3F;
	 */
	OTHERS &= ~0xC0;

	/* 3. Wait for the system colck mode to be asynchronous mode */
	/* Get the SYNC mode from OTHERS register */
	while (1) {
		if ((OTHERS & 0x00000F00) == 0)
			break;
	}

	/* 4. Set the Fout */
	/* Set the APLL Fout */
	switch (mode) {
	case 1:
		/* Fout(APLL) = 266 * 12 / (3 * (2 ^ 2)) = 266 MHz */
		APLL_CON = (1 << 31) | (266 << 16) | (3 << 8) | 2;
		break;
	case 2:
		/* Fout(APLL) = 400  * 12  / (3    * (2 ^ 2)) = 400 MHz */
		APLL_CON = (1 << 31) | (400 << 16) | (3 << 8) | 2;
		break;
	case 4:
		/* Fout(APLL) = 333  * 12  / (3    * (2 ^ 1)) = 666 MHz */
		APLL_CON = (1 << 31) | (333 << 16) | (3 << 8) | 1;
		break;
	default:
		/* Case 3 is default mode */
		/* Fout(APLL) = 12 * 266 / (3 * 2 ^ 1) = 532 MHz */
		APLL_CON = (1 << 31) | (266 << 16) | (3 << 8) | 1;
		break;
	}

	/* Set the MPLL Fout */
	/* Fout(MPLL) = 12 * 266 / (3 * 2 ^ 2) = 266 MHz */
	MPLL_CON = (1 << 31) | (266 << 16) | (3 << 8) | 2;


	/* We don't set EPLL */


	/* 5. Set the clock divider */
	/*
	 * ARM_RATIO = 0, MPLL_RATIO = 0, HCLKX2_RATIO = 0, HCLK_RATIO = 1
	 * PCLK_RATIO = 3, SECUR_RATIO = 3, JPEG_RATIO = 3
	 * XXXX_0011_XXXX_11XX_0011_0001_0000_0000
	 */
	CLK_DIV0 = (CLK_DIV0 & 0xF0F30000) | 0x030C3100;

	/* 6. Set the clock source */
	/* Set to use APLL and MPLL */
	CLK_SRC = 0x03;
}
