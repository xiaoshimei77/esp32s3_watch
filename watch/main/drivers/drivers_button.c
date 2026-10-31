#include "drivers_button.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "string.h"
#include "esp_log.h"
#include <stdlib.h>

//链表要先创建一个头指针
static button_info_t *button_head = NULL;
//定时器句柄
static esp_timer_handle_t button_timer_handle=NULL;
//定时器回调函数
static void button_timer_callback(void* arg);
//定时器全局标志位 防止重复创建
static bool is_timer_running=false;
//消抖过滤时间
#define FILITER_TIMER   20

//button register funtion 
esp_err_t button_event_set(button_config_t *cfg) 
{
    //malloc and Insert into(插入) Linked List
    button_info_t* btn=(button_info_t*)malloc(sizeof(button_info_t));
    if(btn==NULL) 
    {
        ESP_LOGE("button_event_set", "malloc failed");
        return ESP_FAIL;
    }
    memset(btn,0,sizeof(button_info_t));
    memcpy(&btn->btn_cfg,cfg,sizeof(button_config_t));
    if(button_head==NULL)
    {
        button_head=btn;
    }
    else
    {
        button_info_t* info=button_head;
        while(info->next!=NULL)
            info=info->next; 
        info->next=btn; 
    }
    if(is_timer_running==false)
    {
        //creat a timer
        static int button_interval=5;
        esp_timer_create_args_t button_timer = {
            .name = "button_timer",
            .arg=(void *)button_interval,
            .callback=button_timer_callback,
            .dispatch_method=ESP_TIMER_TASK 
        };
        esp_timer_create(&button_timer, &button_timer_handle);

        //start the timer
        esp_timer_start_periodic(button_timer_handle,5000);
        is_timer_running=true;
    }
    return ESP_OK;
}

//Execute every five milliseconds
static void button_timer_callback(void* arg)
{
   button_info_t* btn_info=button_head;
   int interval= (int) arg;
    for (; btn_info;btn_info=btn_info->next)
    {  
        int gpio_num=btn_info->btn_cfg.pin;
        switch (btn_info->state)
        {
            case BUTTON_RELEASE:
                if(btn_info->btn_cfg.get_level_cb(gpio_num)==(btn_info->btn_cfg.active_level))
                {
                    btn_info->state=BUTTON_PRESS;
                    btn_info->press_cnt += interval;
                }
                break;
            case BUTTON_PRESS:
                if(btn_info->btn_cfg.get_level_cb(gpio_num)==(btn_info->btn_cfg.active_level))
                {
                    btn_info->press_cnt += interval;
                    if(btn_info->press_cnt>FILITER_TIMER)
                    {
                        btn_info->state=BUTTON_WAIT_RELEASE;
                    }
                }
                else
                {
                    btn_info->press_cnt = 0;
                    btn_info->state=BUTTON_RELEASE;
                }
                break;
            case BUTTON_WAIT_RELEASE:
                if(btn_info->btn_cfg.get_level_cb(gpio_num)!=btn_info->btn_cfg.active_level)
                { 
                   
                    if (btn_info->btn_cfg.short_press_cb)
                        btn_info->btn_cfg.short_press_cb(gpio_num);
                    btn_info->press_cnt = 0;
                    btn_info->state=BUTTON_RELEASE;
                }
                else
                {
                    btn_info->press_cnt += interval;
                    btn_info->state=BUTTON_HOLD;
                }
                break;
            case BUTTON_HOLD:
                if(btn_info->btn_cfg.get_level_cb(gpio_num)==btn_info->btn_cfg.active_level)
                {
                    btn_info->press_cnt += interval;
                    if(btn_info->press_cnt >= btn_info->btn_cfg.long_press_time)
                    {
                        if(btn_info->btn_cfg.long_press_cb)
                            btn_info->btn_cfg.long_press_cb(gpio_num);
                        btn_info->state=BUTTON_LONG_PRESS_HOLD;
                    }
                }
                 else
                  {
                        if(btn_info->btn_cfg.short_press_cb)
                            btn_info->btn_cfg.short_press_cb(gpio_num);
                        btn_info->press_cnt = 0;
                        btn_info->state=BUTTON_RELEASE;
                 } 
                break;
            case BUTTON_LONG_PRESS_HOLD:
                if (btn_info->btn_cfg.get_level_cb(gpio_num)!=btn_info->btn_cfg.active_level)
                {
                   
                    btn_info->press_cnt = 0;
                    btn_info->state=BUTTON_RELEASE;
                }
                break;
            default:
                ESP_LOGI("button", "button state error");
                break;
        }
    }
}

