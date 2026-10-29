/*
 * CST816S 电容触摸 (I2C) — 底层驱动
 *
 * 寄存器 (参考 LovyanGFX Touch_CST816S / CST816S register declaration):
 *   0x02 : 触摸点数 (低 4 位)
 *   0x03 : X 高位 (低 4 位有效)
 *   0x04 : X 低位
 *   0x05 : Y 高位 (低 4 位有效)
 *   0x06 : Y 低位
 *   0xAA : TP厂家ID
 * 一次读取 0x02 起 6 个字节即可拿到点数与坐标。
 */

#include "drivers_touch.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"

#define TAG "TOUCH"

#define TOUCH_I2C_PORT   I2C_NUM_0
#define PIN_TOUCH_SDA    3
#define PIN_TOUCH_SCL    4
#define PIN_TOUCH_INT    2
#define TOUCH_I2C_ADDR   0x15
#define TOUCH_I2C_FREQ   400000

#define CST816S_REG_TOUCH   0x02

static i2c_master_bus_handle_t s_bus    = NULL;
static i2c_master_dev_handle_t s_dev    = NULL;

esp_err_t touch_init(void)
{
    /* INT 脚: 输入 (保留给后续中断方案, 当前轮询 I2C) */
    gpio_config_t int_conf = {
        .pin_bit_mask = (1ULL << PIN_TOUCH_INT),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&int_conf), TAG, "int gpio config failed");

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port          = TOUCH_I2C_PORT,
        .sda_io_num        = PIN_TOUCH_SDA,
        .scl_io_num        = PIN_TOUCH_SCL,
        .clk_source        = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &s_bus), TAG, "i2c bus init failed");

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = TOUCH_I2C_ADDR,
        .scl_speed_hz    = TOUCH_I2C_FREQ,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev),
                        TAG, "add touch device failed");
 
    //这里读取一下0xaa 读出产品id代表设备存在   
    uint8_t write_buffer[1]; 
    uint8_t read_buffer[1];
    write_buffer[0] = 0xAA;
    esp_err_t ret ;
    ret = i2c_master_transmit_receive(s_dev, write_buffer, 1, read_buffer, 1, 50);
    if(ret == ESP_OK) 
    {
        ESP_LOGI(TAG, "\tChip ID: 0x%02x", read_buffer[0]);
    } else {
        ESP_LOGW(TAG, "读 ID 失败（0x%02X）: %s —— 检查焊接/I2C 上拉",TOUCH_I2C_ADDR, esp_err_to_name(ret));    
    }
    
    ESP_LOGI(TAG, "CST816S ready (SDA=%d SCL=%d INT=%d addr=0x%02X)",
             PIN_TOUCH_SDA, PIN_TOUCH_SCL, PIN_TOUCH_INT, TOUCH_I2C_ADDR);
    return ESP_OK;
}

bool touch_read(int *x, int *y)
{
    if (!s_dev) {
        return false;
    }

    uint8_t reg = CST816S_REG_TOUCH;
    uint8_t buf[6] = {0}; //0x02 起 6 个字节
    // 触摸未按下时 CST816S 可能不应答 I2C, 失败按"未触摸"处理
    if(i2c_master_transmit_receive(s_dev, &reg, 1, buf, sizeof(buf), 50) != ESP_OK)
    {
        return false;
    }
   
    uint8_t points= buf[0] & 0x0f; //触摸点数 (低 4 位)
    if (points == 0) {
        return false; //0：无手指 1：1个手指
    }

    //凭借触摸点 xy 坐标
    int16_t tx = buf[2] | ((buf[1] & 0x0F) << 8);
    int16_t ty = buf[4] | ((buf[3] & 0x0F) << 8);

    //防御性处理，避免越界
    if (tx < 0) tx = 0;
    if (ty < 0) ty = 0;
    if (tx > TOUCH_X_MAX - 1) tx = TOUCH_X_MAX - 1;
    if (ty > TOUCH_Y_MAX - 1) ty = TOUCH_Y_MAX - 1;

    if (x) *x = tx;
    if (y) *y = ty;
    return true;
}






