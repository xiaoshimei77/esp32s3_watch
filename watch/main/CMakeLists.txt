idf_component_register(SRCS "main.c" 
                            "drivers/drivers_st7789.c"
                            "drivers/drivers_touch.c"
                            "drivers/drivers_sdmmc.c"
                            "drivers/drivers_button.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_gpio  esp_lcd esp_driver_ledc esp_driver_spi esp_driver_i2c
                             fatfs  esp_driver_sdmmc esp_timer)
