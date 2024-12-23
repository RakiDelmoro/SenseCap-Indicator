#include "esp_log.h"
#include "bsp_board.h"
#include "lv_port.h"
#include "ui.h"
#include "mqtt_client.h"


static void massage_switch_cb(lv_event_t *e) {
    static bool switch_state = false;
    switch_state = !switch_state;
    ESP_LOGI("switch", "click, cur st: %s mode", switch_state ? "on" : "off");
}

static void update_water_tank_arc(sensor_read){
    // Clean previous UI
    lv_obj_clean(ui_Arc2);
    lv_obj_del(ui_Arc2);

    lv_obj_t *new_ui_Arc2 = lv_arc_create(ui_water_tank);
    lv_obj_set_width( new_ui_Arc2, 169);
    lv_obj_set_height( new_ui_Arc2, 169);
    lv_obj_set_x( new_ui_Arc2, 6 );
    lv_obj_set_y( new_ui_Arc2, -21 );
    lv_obj_set_align( new_ui_Arc2, LV_ALIGN_CENTER );
    lv_arc_set_value(new_ui_Arc2, sensor_read);
    lv_obj_set_style_arc_width(new_ui_Arc2, 20, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(new_ui_Arc2, false, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(new_ui_Arc2, lv_color_hex(0x31E6FA), LV_PART_INDICATOR | LV_STATE_DEFAULT );
    lv_obj_set_style_arc_opa(new_ui_Arc2, 255, LV_PART_INDICATOR| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(new_ui_Arc2, 20, LV_PART_INDICATOR| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(new_ui_Arc2, false, LV_PART_INDICATOR| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(new_ui_Arc2, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(new_ui_Arc2, 0, LV_PART_KNOB| LV_STATE_DEFAULT);

    lv_label_set_text(ui_Septic_Level_Value, sensor_read);


}

static void update_septic_tank_arc(sensor_read){
    // Clean previous UI
    lv_obj_clean(ui_Arc3);
    lv_obj_del(ui_Arc3);

    lv_obj_t *new_ui_Arc3 = lv_arc_create(ui_septic_tank);
    lv_obj_set_width( new_ui_Arc3, 169);
    lv_obj_set_height( new_ui_Arc3, 169);
    lv_obj_set_x( new_ui_Arc3, 6 );
    lv_obj_set_y( new_ui_Arc3, -21 );
    lv_obj_set_align( new_ui_Arc3, LV_ALIGN_CENTER );
    lv_arc_set_value(new_ui_Arc3, sensor_read);
    lv_obj_set_style_arc_width(new_ui_Arc3, 20, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(new_ui_Arc3, false, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(new_ui_Arc3, lv_color_hex(0x31E6FA), LV_PART_INDICATOR | LV_STATE_DEFAULT );
    lv_obj_set_style_arc_opa(new_ui_Arc3, 255, LV_PART_INDICATOR| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(new_ui_Arc3, 20, LV_PART_INDICATOR| LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(new_ui_Arc3, false, LV_PART_INDICATOR| LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(new_ui_Arc3, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(new_ui_Arc3, 0, LV_PART_KNOB| LV_STATE_DEFAULT);

    lv_label_set_text(ui_Water_Level_Value, sensor_read);

}

void app_main(void) {
    ESP_ERROR_CHECK(bsp_board_init());
    lv_port_init();
    lv_port_sem_take();
    ui_init();
    lv_port_sem_give();
    
    int water_level1 = 10;
    int water_level2 = 15;
    // update_septic_tank_arc(water_level1);
    // update_water_tank_arc(water_level2);
}