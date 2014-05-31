/*******************************************************************************
* File: lcd.h
* 
* Description:
*
* By weiqiangdragonite@gmail.com
*
* Date: 2013-8-16
*******************************************************************************/

#ifndef LCD_H
#define LCD_H

#include "types.h"

#define FONT_WIDTH  8
#define FONT_HEIGHT 16

#define ENABLE_LCD		1


/*

4.3 - 480 * 272

         272
    (0,0)
    +----------+
    |          |
    |          |
    |          |
    |          |
    |          |
    |          | 480  -> config_lcd(272, 480, SCREEN_VERTICAL);
    |          |
  f |          |
  o |          |
  r |          |
    +----------+(271,479)


        480
    |(0,0)
  --+--------------------------> x
    |                    |
    |                    | 272  -> config_lcd(480, 272, SCREEN_HOIZONTAL);
    |                    |
    |                    |
    |--------------------+(479,271)
    |              forlinx
    V
    y


1个字符大小为 8 x 16 像素
本显示屏宽480，可显示60列字符，高为272，可显示17行字符，共显示60 * 17 = 1020 个字符
列：0 ~ 59
行：0 ~ 16
显示的字符串：


*/


// the default screen orientation is horizontal
#define SCREEN_HOIZONTAL    0
#define SCREEN_VERTICAL     1

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int orientation;
    unsigned int x_position;
    unsigned int y_position;
} LCD_CFG;

LCD_CFG lcd_cfg;

//#define     SCREEN_SIZE_X       480
//#define     SCREEN_SIZE_Y       272

// default, recommend not changed
#define     FRAME_BUFFER        0x5FA00000

//unsigned int FRAME_BUFFER[480 * 272];


//#define     LINEVAL             (SCREEN_SIZE_Y - 1)
//#define     HOZVAL              (SCREEN_SIZE_X - 1)

#define     COLOR_BLACK         0x000000
#define     COLOR_WHITE         0xFFFFFF
#define     COLOR_RED           0xFF0000
#define     COLOR_GREEN         0x00FF00
#define     COLOR_BLUE          0x0000FF
#define     COLOR_YELLOW        0xFFFF00

// 
unsigned short curr_row;
unsigned short curr_col;


unsigned int ENABLE_LCD_POWER;
unsigned int ENABLE_LCD_BL;
unsigned int ENABLE_LCD_DIS;

/* function prototypes */
void config_lcd(unsigned int width, unsigned int height, unsigned int orientation);
void init_lcd(void);
void set_lcd_orientation(unsigned int orientation);
int set_lcd_cursor(unsigned int x, unsigned int y);

void enable_lcd_power(void);
void disable_lcd_power(void);
void enable_lcd_backLight(void);
void disable_lcd_backLight(void);
void enable_lcd_display(void);
void disable_lcd_display(void);

void change_bg_color(void);
void display_bg_color(unsigned int color);

int lcd_write_pixel(unsigned int x, unsigned int y, unsigned int color);
unsigned int lcd_get_pixel(unsigned int x, unsigned int y);

void lcd_display_char(unsigned int x, unsigned int y,
                      unsigned int font_color, unsigned int bg_color, char ch);
void lcd_display_str(unsigned int x, unsigned int y,
                     unsigned int font_color, unsigned int bg_color, char *str);
void lcd_display_string(int line, int column, unsigned int font_color,
                        unsigned int bg_color, char *str);

void lcd_clear_line(int line, unsigned int bg_color);


void lcd_create_traffic_bg(void);
void lcd_display_south_north_light(u8 status);
void lcd_display_east_west_light(u8 status);
void lcd_display_people_line(u8 direction, u8 status);

void lcd_display_int(int line, int column, unsigned int font_color,
		unsigned int bg_color, int value);

#endif	/* LCD_H */
