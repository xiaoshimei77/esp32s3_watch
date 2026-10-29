#ifndef _DRIVERS_TOUCH_H_
#define _DRIVERS_TOUCH_H_

#include <stdio.h>
#include <stdbool.h>
#include "esp_err.h"

/* 触摸原始坐标范围 */
#define TOUCH_X_MAX   240
#define TOUCH_Y_MAX   280

esp_err_t touch_init(void);
bool touch_read(int *x, int *y);


#endif

