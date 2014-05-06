/*******************************************************************************
* File: lcd.c
* 
* Description:
*
* By weiqiangdragonite@gmail.com
*
* Date: 2014
*******************************************************************************/


#include "s3c6410.h"
#include "lcd.h"
#include "types.h"
#include "font_8x16.h"



void config_lcd(unsigned int width, unsigned int height, unsigned int orientation)
{
    // version 1
    //lcd_cfg.width = width;
    //lcd_cfg.height = height;
    //lcd_cfg.orientation = orientation;
    
    // version 2
    if (orientation == SCREEN_HOIZONTAL) {
        lcd_cfg.width = width;
        lcd_cfg.height = height;
    } else if (orientation == SCREEN_VERTICAL) {
        lcd_cfg.width = height;
        lcd_cfg.height = width;
    }
    lcd_cfg.orientation = orientation;
    
    return;
}


void init_lcd(void)
{
    /** 1. 设置LCD的相关GPIO引脚 */
    // VD0 ~ VD23的引脚为GPI0 ~ GPI15，GPJ0 ~ GPJ7
    // LDVEN、LVSYNC、LHSYNC、LVCLK的引脚为GPJ8 ~ GPJ11
    GPICON = 0xAAAAAAAA;
    GPJCON = 0xAAAAAA;
    
    // GPF14用作背光使能信号，设为输出模式
    // GPE0用作LCD的电源使能信号，设为输出模式
    GPFCON &= ~(3 << 28);
    GPFCON |= (1 << 28);
    GPECON &= ~(0xF);
    GPECON |= 1;
    
    /** 2. 初始化LCD控制器 */
    // PROGRAMMER'S MODEL (P-492)
    // 设置MIFPCON的SEL_BYPASS[3]为Normal mode
    MIFPCON &= ~(1 << 3);
    
    // 设置SPCON的LCD_SEL[1:0]为RGB I/F Style
    SPCON = (SPCON & (~3)) | 1;
    
    // 设置VIDCON0控制器，设置图像输出格式等属性
    VIDCON0 &= ~((3 << 26) | (3 << 17) | (0xFF << 6) | 0x1F);
    VIDCON0 |= ((13 << 6) | (1 << 4));
    
    // 设置VIDCON1控制器，设置HSYNC、VSYNC、VCLK、VDEN
    VIDCON1 &= ~(0xF << 4);
    VIDCON1 |= (0x7 << 4);
    
    // 设置VIDTCON控制器的时间参数，行列数
    VIDTCON0 = (1 << 16) | (1 << 8) | 9;
    VIDTCON1 = (1 << 16) | (1 << 8) | 40;
    VIDTCON2 = ((lcd_cfg.height - 1) << 11) | (lcd_cfg.width - 1);
    
    // 设置WINCON0的输出像素格式
    // use 16 BPP - RGB 565 - 0101
    // 24 BPP - 1011
    WINCON0 &= ~(0xF << 2);
    WINCON0 |= (0xB << 2);
    
    // 设置左上角和右下角坐标
    VIDOSD0A = 0;
    VIDOSD0B = ((lcd_cfg.width - 1) << 11) | (lcd_cfg.height - 1);
    
    // 设置图像大小
    VIDOSD0C = lcd_cfg.width * lcd_cfg.height;
    
    // 设置显存起始地址
    VIDW00ADD0B0 = (int) &FRAME_BUFFER[0];
    // 设置显存结束地址
    VIDW00ADD1B0 = lcd_cfg.width * 4 * lcd_cfg.height;
    
    // 
    enable_lcd_power();
    enable_lcd_backLight();
    enable_lcd_display();
    display_bg_color(COLOR_WHITE);

    curr_row = 0;
    curr_col = 0;
    
    return;
}

void enable_lcd_power(void)
{
    // 输出高电平，使能电源
    // 看不出有作用
    GPEDAT |= 1;
    
    ENABLE_LCD_POWER = TRUE;
    
    return;
}

void disable_lcd_power(void)
{
    // 输出低电平
    // 看不出有作用
    GPEDAT &= ~1;
    
    ENABLE_LCD_POWER = FALSE;
    
    return;
}

void enable_lcd_backLight(void)
{
    GPFDAT |= (1<<14);
    
    ENABLE_LCD_BL = TRUE;
    
    return;
}

void disable_lcd_backLight(void)
{
    GPFDAT &= ~(1<<14);
    
    ENABLE_LCD_BL = FALSE;
    
    return;
}

void enable_lcd_display(void)
{
    VIDCON0 |= 3;
	WINCON0 |= 1;
    
    ENABLE_LCD_DIS = TRUE;
    
    return;
}

void disable_lcd_display(void)
{
    VIDCON0 &= ~3;
	WINCON0 &= ~1;
    
    ENABLE_LCD_DIS = FALSE;
    
    return;
}

void change_bg_color(void)
{
    volatile unsigned int *ptr = (volatile unsigned int *) FRAME_BUFFER;
    unsigned int addr = 0;
    
    unsigned int colors[] = { COLOR_BLACK, COLOR_WHITE , COLOR_RED, COLOR_GREEN, COLOR_BLUE };
    static int index = 0;
    
    // display color
    int x, y;
    for (y = 0; y <= lcd_cfg.height; ++y) {
        for (x = 0; x <= lcd_cfg.width; ++x) {
            ptr[addr++] = colors[index];
        }
    }
    
    if (++index == 5) index = 0;
    
    return;
}

void display_bg_color(unsigned int color)
{
    volatile unsigned int *ptr = (volatile unsigned int *) FRAME_BUFFER;
    unsigned int addr = 0;
    
    int x, y;
    for (y = 0; y < lcd_cfg.height; ++y) {
        for (x = 0; x < lcd_cfg.width; ++x) {
            ptr[addr++] = color;
        }
    }
    
    return;
}

/* version 1
int set_lcd_cursor(unsigned int x, unsigned int y)
{
    // check
    if (lcd_cfg.orientation == SCREEN_VERTICAL) {
        if (!(x >= 0 && x < lcd_cfg.height && y >= 0 && y < lcd_cfg.width))
            // error
            return -1;
    } else if (lcd_cfg.orientation == SCREEN_HOIZONTAL) {
        if (!(x >= 0 && x < lcd_cfg.width && y >= 0 && y < lcd_cfg.height))
            // error
            return -2;
    }
    
    if (lcd_cfg.orientation == SCREEN_HOIZONTAL) {
        lcd_cfg.x_position = x;
        lcd_cfg.y_position = y;
    } else if (lcd_cfg.orientation == SCREEN_VERTICAL) {
        lcd_cfg.x_position = y;
        lcd_cfg.y_position = lcd_cfg.height - x - 1;
    }
    
    return 0;
}
*/

// version 2
int set_lcd_cursor(unsigned int x, unsigned int y)
{
    // check
    if (lcd_cfg.orientation == SCREEN_VERTICAL) {
        if (!(x >= 0 && x < lcd_cfg.height && y >= 0 && y < lcd_cfg.width))
            // error
            return -1;
    } else if (lcd_cfg.orientation == SCREEN_HOIZONTAL) {
        if (!(x >= 0 && x < lcd_cfg.width && y >= 0 && y < lcd_cfg.height))
            // error
            return -2;
    }
    
    if (lcd_cfg.orientation == SCREEN_HOIZONTAL) {
        lcd_cfg.x_position = x;
        lcd_cfg.y_position = y;
    } else if (lcd_cfg.orientation == SCREEN_VERTICAL) {
        lcd_cfg.x_position = y;
        lcd_cfg.y_position = lcd_cfg.height - x - 1;
    }
    
    return 0;
}

int lcd_write_pixel(unsigned int x, unsigned int y, unsigned int color)
{
    int res = set_lcd_cursor(x, y);
    if (res < 0) return -1;
    
    volatile unsigned int *ptr = (volatile unsigned int *) FRAME_BUFFER;
    ptr[lcd_cfg.y_position * lcd_cfg.width + lcd_cfg.x_position] = color;
    
    return 0;
}

unsigned int lcd_get_pixel(unsigned int x, unsigned int y)
{
    int res = set_lcd_cursor(x, y);
    if (res < 0) return 0;
    
    volatile unsigned int *ptr = (volatile unsigned int *) FRAME_BUFFER;
    
    return ptr[lcd_cfg.y_position * lcd_cfg.width + lcd_cfg.x_position];
}



void lcd_display_char(unsigned int x, unsigned int y,
                      unsigned int font_color, unsigned int bg_color, char ch)
{
    unsigned char font;
    int index = (int) ch;
    int i, j, k;

    for (i = 0; i < FONT_HEIGHT; ++i, ++y) {
        font = courier_font[index][i];
        for (j = 0, k = x; j < FONT_WIDTH; ++j, ++k) {
            if (font & 0x80) lcd_write_pixel(k, y, font_color);
            else lcd_write_pixel(k, y, bg_color);
            font <<= 1;
        }
    }
    
    return;
}

void lcd_display_str(unsigned int x, unsigned int y,
                     unsigned int font_color, unsigned int bg_color , char *str)
{
    int i;
    for (i = 0; str[i] != '\0'; ++i) {
        lcd_display_char(x, y, font_color, bg_color , str[i]);
        x += FONT_WIDTH;
    }
    
    return;
}

void lcd_display_string(int line, int column, unsigned int font_color,
                        unsigned int bg_color, char *str)
{
    unsigned int x = column * FONT_WIDTH;
    unsigned int y = line * FONT_HEIGHT;
    
    int i;
    for (i = 0; str[i] != '\0'; ++i) {
        lcd_display_char(x, y, font_color, bg_color, str[i]);
        x += FONT_WIDTH;
    }
    
    return;
}


void
lcd_clear_line(int line, unsigned int bg_color)
{
	if (line >= 17)
		return;

	unsigned int x = 0;
	unsigned int y = line * FONT_HEIGHT;

	int i, j;
	for (j = y; j < FONT_HEIGHT; ++j) {
		for (i = x; i < lcd_cfg.width; ++i) {
			lcd_write_pixel(i, j, bg_color);
		}
	}
}

/*
 * -----------------------------------------------------------------------------
 * -----------------------------------------------------------------------------
 */












