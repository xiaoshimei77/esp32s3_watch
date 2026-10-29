#ifndef _DRIVERS_ST7789_H_
#define _DRIVERS_ST7789_H_

#include <stdint.h>
#include "esp_err.h"
/* 屏幕可见分辨率 (ST7789V, 240x280, st7789是240x320显存 上下有20行看不见，所以顶部 20 行为假像素) */
#define LCD_H_RES   240
#define LCD_V_RES   280
  
esp_err_t lcd_init(void);
void lcd_set_backlight(uint8_t level);
esp_err_t lcd_fill_screen(uint16_t color);
esp_err_t lcd_draw_bitmap_sync(int x_start, int y_start, int x_end, int y_end,
                               const void *color_data);


#endif