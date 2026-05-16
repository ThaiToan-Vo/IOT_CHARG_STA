idf_component_register(SRCS espnow_init.c
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES main
                    REQUIRES esp_wifi  esp_event nvs_flash driver
                            ) # "."