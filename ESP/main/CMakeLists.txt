idf_component_register(SRCS "main.c"  
                            "process.c" 
                            "task_read_data.c" 
                            "task_process_data.c" 
                            "task_oled.c" 
                    INCLUDE_DIRS "."
                    REQUIRES ssd1306 
                            hardware_conf 
                            esp_timer)