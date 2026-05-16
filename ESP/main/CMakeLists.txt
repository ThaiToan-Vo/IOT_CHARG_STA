idf_component_register(SRCS "task_handshake.c" "task_control.c" "task_espnow_send.c" "main.c"  
                            "process.c" 
                            "task_read_data.c" 
                            "task_process_data.c" 
                            "task_oled.c" 
                           
                    INCLUDE_DIRS "."
                    REQUIRES ssd1306 
                            hardware_conf
                            app_espnow 
                            esp_timer
                            common)