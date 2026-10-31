#ifndef __DRIVERS_BUTTON_H
#define __DRIVERS_BUTTON_H

#include "driver/gpio.h"
#include "esp_err.h"

//button callback
typedef void (*button_press_cb_t)(int gpio);

//get button level
typedef int (*button_getlevel_get_t)(int gpio);

//gpio config
typedef struct
{
    int pin;                            //gpio pin
    int active_level;                   //active level
    int long_press_time;                //long press time
    button_press_cb_t short_press_cb;   //short press callback
    button_press_cb_t long_press_cb;    //long press callback
    button_getlevel_get_t get_level_cb; //get io level callback
} button_config_t;

typedef enum
{ 
    BUTTON_RELEASE = 0,
    BUTTON_PRESS,
    BUTTON_HOLD,
    BUTTON_LONG_PRESS_HOLD,
    BUTTON_WAIT_RELEASE
} button_state;

//button state message
typedef struct Button_info
{
    button_config_t btn_cfg;    //button config
    button_state state;         //button state
    int press_cnt;              //press count
    struct Button_info *next;   //next button 用链表容易索引和创建
} button_info_t;

// button register 
esp_err_t button_event_set(button_config_t *cfg);

#endif 
