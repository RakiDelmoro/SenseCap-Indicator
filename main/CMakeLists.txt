set(UI_DIR ./ui)
file(GLOB_RECURSE UI_SOURCES ${UI_DIR}/*.c)

idf_component_register(
    SRCS "main.c"
         "lv_port.c"
         "wifi_manager.c"
         "mqtt_manager.c"
         "ui_handlers.c"
         ${UI_SOURCES}
    INCLUDE_DIRS "." ${UI_DIR})
