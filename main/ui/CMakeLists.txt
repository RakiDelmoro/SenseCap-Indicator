SET(SOURCES screens/ui_Water_Level_Sensor_and_buttons.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c)

add_library(ui ${SOURCES})
