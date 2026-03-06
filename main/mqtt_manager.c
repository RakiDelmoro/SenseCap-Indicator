#include "mqtt_manager.h"
#include "app_config.h"
#include "ui.h"
#include "cJSON.h"
#include "esp_log.h"
#include <string.h>

#define TAG "MQTT_MANAGER"

static esp_mqtt_client_handle_t s_client = NULL;

static void handle_bright_switch_state(const char *data, int data_len)
{
    if (data_len < 1) return;
    
    char command = data[0];
    if (command == '1') {
        lv_obj_add_state(ui_BrightSwitch, LV_STATE_CHECKED);
    } else if (command == '0') {
        lv_obj_clear_state(ui_BrightSwitch, LV_STATE_CHECKED);
    }
    
    char state[2] = {command, '\0'};
    esp_mqtt_client_publish(s_client, "sensecap/bright_switch", state, 1, 1, 0);
}

static void handle_relax_switch_state(const char *data, int data_len)
{
    if (data_len < 1) return;
    
    char command = data[0];
    if (command == '1') {
        lv_obj_add_state(ui_RelaxSwitch, LV_STATE_CHECKED);
    } else if (command == '0') {
        lv_obj_clear_state(ui_RelaxSwitch, LV_STATE_CHECKED);
    }
    
    char state[2] = {command, '\0'};
    esp_mqtt_client_publish(s_client, "sensecap/relax_switch", state, 1, 1, 0);
}

static void handle_sensor_data(const char *data, int data_len)
{
    char mqtt_data[50];
    if (data_len >= sizeof(mqtt_data)) data_len = sizeof(mqtt_data) - 1;
    strncpy(mqtt_data, data, data_len);
    mqtt_data[data_len] = '\0';
    
    cJSON *json_data = cJSON_Parse(mqtt_data);
    if (json_data == NULL) return;
    
    cJSON *water_level2 = cJSON_GetObjectItem(json_data, "water_level");
    if (water_level2) {
        int level = (water_level2->valueint - 400) * 100 / (1260 - 400);
        if (level < 0) level = 0;
        if (level > 100) level = 100;
        
        lv_arc_set_value(ui_WaterTankArc, level);
        char buffer[8];
        snprintf(buffer, sizeof(buffer), "%d%%", level);
        lv_label_set_text(ui_WaterLevel, buffer);
    }
    
    cJSON_Delete(json_data);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, 
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");
            esp_mqtt_client_subscribe(s_client, TOPIC_BRIGHT_STATE, 1);
            esp_mqtt_client_subscribe(s_client, TOPIC_RELAX_STATE, 1);
            esp_mqtt_client_subscribe(s_client, TOPIC_SENSORS, 1);
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT disconnected");
            break;
            
        case MQTT_EVENT_DATA:
            if (strncmp(event->topic, TOPIC_BRIGHT_STATE, event->topic_len) == 0) {
                handle_bright_switch_state(event->data, event->data_len);
            } else if (strncmp(event->topic, TOPIC_RELAX_STATE, event->topic_len) == 0) {
                handle_relax_switch_state(event->data, event->data_len);
            } else if (strncmp(event->topic, TOPIC_SENSORS, event->topic_len) == 0) {
                handle_sensor_data(event->data, event->data_len);
            }
            break;
            
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error occurred");
            break;
    }
}

esp_err_t mqtt_manager_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URL,
        .credentials.client_id = MQTT_CLIENT_ID,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
    };
    
    s_client = esp_mqtt_client_init(&mqtt_cfg);
    if (s_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return ESP_FAIL;
    }
    
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    
    esp_err_t err = esp_mqtt_client_start(s_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(err));
        esp_mqtt_client_destroy(s_client);
        s_client = NULL;
        return err;
    }
    
    ESP_LOGI(TAG, "MQTT manager initialized");
    return ESP_OK;
}

esp_err_t mqtt_publish(const char *topic, const char *data, int qos)
{
    if (s_client == NULL || topic == NULL || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int msg_id = esp_mqtt_client_publish(s_client, topic, data, strlen(data), qos, 0);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t mqtt_subscribe(const char *topic, int qos)
{
    if (s_client == NULL || topic == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int msg_id = esp_mqtt_client_subscribe(s_client, topic, qos);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

esp_mqtt_client_handle_t mqtt_manager_get_client(void)
{
    return s_client;
}
