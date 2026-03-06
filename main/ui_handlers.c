#include "ui_handlers.h"
#include "ui.h"
#include "esp_log.h"

#define TAG "UI_HANDLERS"

static bool s_bright_state = false;
static bool s_relax_state = false;

static void bright_switch_cb(lv_event_t *e)
{
    esp_mqtt_client_handle_t mqtt_client = lv_event_get_user_data(e);
    if (mqtt_client == NULL) return;
    
    s_bright_state = !s_bright_state;
    ESP_LOGI(TAG, "Bright switch: %s", s_bright_state ? "ON" : "OFF");
    
    if (s_bright_state) {
        lv_obj_add_state(ui_BrightSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_BrightSwitch, LV_STATE_CHECKED);
    }
    
    char data[2] = {s_bright_state ? '1' : '0', '\0'};
    esp_mqtt_client_publish(mqtt_client, "sensecap/bright_switch", data, 1, 1, 0);
}

static void relax_switch_cb(lv_event_t *e)
{
    esp_mqtt_client_handle_t mqtt_client = lv_event_get_user_data(e);
    if (mqtt_client == NULL) return;
    
    s_relax_state = !s_relax_state;
    ESP_LOGI(TAG, "Relax switch: %s", s_relax_state ? "ON" : "OFF");
    
    if (s_relax_state) {
        lv_obj_add_state(ui_RelaxSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_RelaxSwitch, LV_STATE_CHECKED);
    }
    
    char data[2] = {s_relax_state ? '1' : '0', '\0'};
    esp_mqtt_client_publish(mqtt_client, "sensecap/relax_switch", data, 1, 1, 0);
}

void ui_setup_callbacks(esp_mqtt_client_handle_t mqtt_client)
{
    lv_obj_add_event_cb(ui_BrightSwitch, bright_switch_cb, LV_EVENT_CLICKED, mqtt_client);
    lv_obj_add_event_cb(ui_RelaxSwitch, relax_switch_cb, LV_EVENT_CLICKED, mqtt_client);
    ESP_LOGI(TAG, "UI callbacks registered");
}

bool ui_bright_switch_toggle(esp_mqtt_client_handle_t mqtt_client)
{
    if (mqtt_client == NULL) return s_bright_state;
    
    s_bright_state = !s_bright_state;
    
    if (s_bright_state) {
        lv_obj_add_state(ui_BrightSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_BrightSwitch, LV_STATE_CHECKED);
    }
    
    char data[2] = {s_bright_state ? '1' : '0', '\0'};
    esp_mqtt_client_publish(mqtt_client, "sensecap/bright_switch", data, 1, 1, 0);
    
    return s_bright_state;
}

bool ui_relax_switch_toggle(esp_mqtt_client_handle_t mqtt_client)
{
    if (mqtt_client == NULL) return s_relax_state;
    
    s_relax_state = !s_relax_state;
    
    if (s_relax_state) {
        lv_obj_add_state(ui_RelaxSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(ui_RelaxSwitch, LV_STATE_CHECKED);
    }
    
    char data[2] = {s_relax_state ? '1' : '0', '\0'};
    esp_mqtt_client_publish(mqtt_client, "sensecap/relax_switch", data, 1, 1, 0);
    
    return s_relax_state;
}

void ui_update_water_tank(int percentage)
{
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;
    
    lv_arc_set_value(ui_WaterTankArc, percentage);
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d%%", percentage);
    lv_label_set_text(ui_WaterLevel, buffer);
}
