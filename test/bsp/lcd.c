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
    VIDW00ADD0B0 = FRAME_BUFFER;
    // 设置显存结束地址，每个像素4个字节
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

	int end = y + FONT_HEIGHT;
	for (; y < end; ++y) {
		for (x = 0; x < lcd_cfg.width; ++x) {
			lcd_write_pixel(x, y, bg_color);
		}
	}
}



void *
lcd_memcpy(void *dest, const void *src, size_t n)
{
	int *tmp = dest;
	const int *s = src;

	/* Copy 4 bytes everytimes */
	while (n--) {
		*tmp = *s;
		++tmp;
		++s;
	}

	return dest;	
}

/*

480 x 272
+-------------++-------------------------------------------------------------
|             ||
|             ||
|             ||
|             ||
+=============++(120,68)
|                                   |
|                                   |
|                                   |
|   -------                         |
|               +--+                |
|   -------     |  |                |
|               |  |                |
|   -------     +--+                |
|                                   |
|                                   |
|                                   |(240,136)
+-----------------------------------+--------------------------------------




-----------------------------------------------------------------------------

*/


void
lcd_create_traffic_bg(void)
{

	display_bg_color(COLOR_WHITE);

	int x, y;

	/* 横 */
	for (x = 0; x < 170; ++x) {
		lcd_write_pixel(x, 66, COLOR_BLACK);
		lcd_write_pixel(x, 136, COLOR_BLACK);
		lcd_write_pixel(x, 206, COLOR_BLACK);
	}

	for (x = 310; x < 480; ++x) {
		lcd_write_pixel(x, 66, COLOR_BLACK);
		lcd_write_pixel(x, 136, COLOR_BLACK);
		lcd_write_pixel(x, 206, COLOR_BLACK);
	}

	/* 竖 */
	for (y = 0; y < 66; ++y) {
		lcd_write_pixel(170, y, COLOR_BLACK);
		lcd_write_pixel(240, y, COLOR_BLACK);
		lcd_write_pixel(310, y, COLOR_BLACK);
	}

	for (y = 206; y < 272; ++y) {
		lcd_write_pixel(170, y, COLOR_BLACK);
		lcd_write_pixel(240, y, COLOR_BLACK);
		lcd_write_pixel(310, y, COLOR_BLACK);
	}
}

void
lcd_display_south_north_light(u8 status)
{
	int x, y, z;
	unsigned int color;

	/* 显示绿灯 */
	if (status == 1)
		color = COLOR_GREEN;
	/* 显示红灯 */
	else if (status == 0)
		color = COLOR_RED;
	/* 显示黄灯 */
	else if (status == 2)
		color = COLOR_YELLOW;


	/* 显示灯 */
	for (y = 66; y < 86; ++y) {
		for (x = 250; x < 300; ++x)
			lcd_write_pixel(x, y, color);
	}

	for (y = 186; y < 206; ++y) {
		for (x = 180; x < 230; ++x)
			lcd_write_pixel(x, y, color);
	}

	/* 线 */
	if (status == 1) {
		for (y = 70; y < 160; ++y)
			lcd_write_pixel(205, y, COLOR_GREEN);

		for (y = 110; y < 200; ++y)
			lcd_write_pixel(275, y, COLOR_GREEN);
	} else {
		for (y = 70; y < 160; ++y)
			lcd_write_pixel(205, y, COLOR_WHITE);

		for (y = 110; y < 200; ++y)
			lcd_write_pixel(275, y, COLOR_WHITE);
	}

	/* 箭头 */
	if (status == 1) {
		for (x = 196, z = 214, y = 150; y < 160; ++y, ++x, --z) {
			lcd_write_pixel(x, y, COLOR_GREEN);
			lcd_write_pixel(z, y, COLOR_GREEN);
		}

		for (x = z = 275, y = 110; y < 120; ++y, --x, ++z) {
			lcd_write_pixel(x, y, COLOR_GREEN);
			lcd_write_pixel(z, y, COLOR_GREEN);
		}
	} else {
		for (x = 196, z = 214, y = 150; y < 160; ++y, ++x, --z) {
			lcd_write_pixel(x, y, COLOR_WHITE);
			lcd_write_pixel(z, y, COLOR_WHITE);
		}

		for (x = z = 275, y = 110; y < 120; ++y, --x, ++z) {
			lcd_write_pixel(x, y, COLOR_WHITE);
			lcd_write_pixel(z, y, COLOR_WHITE);
		}
	}
}

void
lcd_display_east_west_light(u8 status)
{
	int x, y, z;
	unsigned int color;

	/* 显示绿灯 */
	if (status == 1)
		color = COLOR_GREEN;
	/* 显示红灯 */
	else if (status == 0)
		color = COLOR_RED;
	/* 显示黄灯 */
	else if (status == 2)
		color = COLOR_YELLOW;


	/* 显示灯 */
	for (y = 76; y < 126; ++y) {
		for (x = 170; x < 190; ++x)
			lcd_write_pixel(x, y, color);
	}

	for (y = 146; y < 196; ++y) {
		for (x = 290; x < 310; ++x)
			lcd_write_pixel(x, y, color);
	}

	/* 线 */
	if (status == 1) {
		for (x = 210; x < 300; ++x)
			lcd_write_pixel(x, 101, COLOR_GREEN);

		for (x = 180; x < 270; ++x)
			lcd_write_pixel(x, 171, COLOR_GREEN);
	} else {
		for (x = 210; x < 300; ++x)
			lcd_write_pixel(x, 101, COLOR_WHITE);

		for (x = 180; x < 270; ++x)
			lcd_write_pixel(x, 171, COLOR_WHITE);
	}

	/* 箭头 */
	if (status == 1) {
		for (x = 210, y = z = 101; x < 220; ++y, ++x, --z) {
			lcd_write_pixel(x, y, COLOR_GREEN);
			lcd_write_pixel(x, z, COLOR_GREEN);
		}

		for (x = 260, y = 162, z = 180; x < 270; ++y, ++x, --z) {
			lcd_write_pixel(x, y, COLOR_GREEN);
			lcd_write_pixel(x, z, COLOR_GREEN);
		}
	} else {
		for (x = 210, y = z = 101; x < 220; ++y, ++x, --z) {
			lcd_write_pixel(x, y, COLOR_WHITE);
			lcd_write_pixel(x, z, COLOR_WHITE);
		}

		for (x = 260, y = 162, z = 180; x < 270; ++y, ++x, --z) {
			lcd_write_pixel(x, y, COLOR_WHITE);
			lcd_write_pixel(x, z, COLOR_WHITE);
		}
	}
}


void
lcd_display_people_line(u8 direction, u8 status)
{
	int x, y;
	unsigned int color;

	/* 显示绿灯 */
	if (status == 1)
		color = COLOR_GREEN;
	/* 显示红灯 */
	else if (status == 0)
		color = COLOR_RED;

	/* 东西的斑马线 */
	if (direction == 1 || direction == 2) {
		/* 画斑马线 */
		for (x = 120; x < 160; ++x) {
			lcd_write_pixel(x, 76, color);
			lcd_write_pixel(x, 86, color);
			lcd_write_pixel(x, 96, color);
			lcd_write_pixel(x, 106, color);
			lcd_write_pixel(x, 116, color);
			lcd_write_pixel(x, 126, color);

			lcd_write_pixel(x, 146, color);
			lcd_write_pixel(x, 156, color);
			lcd_write_pixel(x, 166, color);
			lcd_write_pixel(x, 176, color);
			lcd_write_pixel(x, 186, color);
			lcd_write_pixel(x, 196, color);
		}

		for (x = 320; x < 360; ++x) {
			lcd_write_pixel(x, 76, color);
			lcd_write_pixel(x, 86, color);
			lcd_write_pixel(x, 96, color);
			lcd_write_pixel(x, 106, color);
			lcd_write_pixel(x, 116, color);
			lcd_write_pixel(x, 126, color);

			lcd_write_pixel(x, 146, color);
			lcd_write_pixel(x, 156, color);
			lcd_write_pixel(x, 166, color);
			lcd_write_pixel(x, 176, color);
			lcd_write_pixel(x, 186, color);
			lcd_write_pixel(x, 196, color);
		}

	/* 南北的斑马线 */
	} else {
		for (y = 20; y < 60; ++y) {
			lcd_write_pixel(180, y, color);
			lcd_write_pixel(190, y, color);
			lcd_write_pixel(200, y, color);
			lcd_write_pixel(210, y, color);
			lcd_write_pixel(220, y, color);
			lcd_write_pixel(230, y, color);

			lcd_write_pixel(250, y, color);
			lcd_write_pixel(260, y, color);
			lcd_write_pixel(270, y, color);
			lcd_write_pixel(280, y, color);
			lcd_write_pixel(290, y, color);
			lcd_write_pixel(300, y, color);
		}

		for (y = 216; y < 256; ++y) {
			lcd_write_pixel(180, y, color);
			lcd_write_pixel(190, y, color);
			lcd_write_pixel(200, y, color);
			lcd_write_pixel(210, y, color);
			lcd_write_pixel(220, y, color);
			lcd_write_pixel(230, y, color);

			lcd_write_pixel(250, y, color);
			lcd_write_pixel(260, y, color);
			lcd_write_pixel(270, y, color);
			lcd_write_pixel(280, y, color);
			lcd_write_pixel(290, y, color);
			lcd_write_pixel(300, y, color);
		}
	}

}



