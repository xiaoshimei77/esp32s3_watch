idf_component_register(SRCS "main.c" 
                            "drivers/drivers_st7789.c"
                            "drivers/drivers_touch.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_gpio  esp_lcd esp_driver_ledc esp_driver_spi esp_driver_i2c)
