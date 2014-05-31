/*******************************************************************************
* File: key.c
* 
* Description:
*   For uC/OS-II(OK6410)
*
* By: weiqiangdragonite@gmail.com
* Updated: 2013-10-14
*******************************************************************************/


#include "s3c6410.h"
#include "key.h"
#include "uart.h"
#include "lcd.h"
#include "types.h"

//extern int green_time;
//extern int red_time;
extern u8 need_create_task;
extern u8 need_delete_task;
//extern prio_t new_prio;

extern u8 need_mem_task;
extern u8 release_mem_task;


void init_key(void)
{
    // key 1 ~ key 6 use GPN0 ~ GPN5
    // set GPNCON[11:0] to be Ext.Interrupt - 1010 1010 1010
    GPNCON &= ~0xFFF;
    GPNCON |= 0xAAA;
    
    // set the GPNPUD[11:0] to disable pull-up/down - 0000 0000 0000
    GPNPUD &= ~0xFFF;
    
    // GPNDAT - key press: low level; key release: high level
    
    // the key source interrupt is EINT0 ~ EINT5
    // set interrupt to High level triggered(release key occur interrupt)
    EINT0CON0 &= ~0xFFF;
    EINT0CON0 |= 0x444;
    
    // enable interrupt in the key source
    EINT0MASK &= ~0x3F;
    
    //--------------------------------------------------------------------------
    
    // enable interrupt in the interrupt controller
    // EINT0 ~ EINT3 is No.0, EINT4 ~ EINT5 is No.1
    VIC0INTENABLE |= 3;
    
    // all the interrupt is set to irq in default
    
    // set the ISR address
    VIC0VECTADDR[0] = (unsigned int) key_isr;
    VIC0VECTADDR[1] = (unsigned int) key_isr;
    
    return;
}

void key_isr(void)
{
    // use EINT0PEND to see if there has interrupt occur and clear interrupt
    unsigned int val = EINT0PEND & 0x3F;
    
    // 
    switch (val) {
        case 1:
            key_1_handler();
            break;
        case 2:
            key_2_handler();
            break;
        case 4:
            key_3_handler();
            break;
        case 8:
            key_4_handler();
            break;
        case 0x10:
            key_5_handler();
            break;
        case 0x20:
            key_6_handler();
            break;
        default:
            break;
    }
    
    // clear interrupt
    EINT0PEND |= 0x3F;
    VIC0ADDRESS = 0;
    
    return;
}

void key_1_handler(void)
{
    //
    uart_print("[key] You press key 1\n");

/*
#ifdef LED_TRAFFIC
	if (green_time >= 100)
		return;

	// Add led time
	green_time += 10;
	red_time = green_time;

	// 右下角显示红绿灯时间 
	lcd_display_string(16, 55, COLOR_WHITE, COLOR_WHITE, "   ");
	lcd_display_int(16, 55, COLOR_BLUE, COLOR_WHITE, green_time);
#endif
*/

	need_create_task = 1;
    
    return;
}

void key_2_handler(void)
{
    //
    uart_print("[key] You press key 2\n");


/*
#ifdef LED_TRAFFIC
	if (green_time <= 30)
		return;

	green_time -= 10;
	red_time = green_time;

	// 右下角显示红绿灯时间 
	lcd_display_string(16, 55, COLOR_WHITE, COLOR_WHITE, "   ");
	lcd_display_int(16, 55, COLOR_BLUE, COLOR_WHITE, green_time);
#endif
*/

	need_delete_task = 1;
    
    return;
}

void key_3_handler(void)
{
    //
    uart_print("[key] You press key 3\n");

	//++new_prio;
    
    return;
}

void key_4_handler(void)
{
    //
    uart_print("[key] You press key 4\n");

	//--new_prio;
    
    return;
}

void key_5_handler(void)
{
    //
    uart_print("[key] You press key 5\n");

	need_mem_task = 1;
    
    return;
}

void key_6_handler(void)
{
    //
    uart_print("[key] You press key 6\n");

	release_mem_task = 1;
    
    return;
}
