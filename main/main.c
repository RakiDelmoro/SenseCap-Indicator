#include "esp_log.h"
#include "bsp_board.h"
#include "lv_port.h"
#include "ui.h"
#include "mqtt_client.h"

#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/event_groups.h"

#define WIFI_SSID      "Zoltu"
#define WIFI_PASSWORD  "1029384756"
#define MAXIMUM_RETRY  5
static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
static const char *TAG = "MQTT_SUBSCRIBER";
static bool wifi_connected = false;

// WiFi event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init_task(void *pvParameters) {
    s_wifi_event_group = xEventGroupCreate();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "Zoltu",
            .password = "1029384756",
        },
    };

    // Set WiFi mode to station mode
    // esp_wifi_set_mode(WIFI_MODE_STA);
    // Set the WiFi configuration to the default STA interface
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    // Start the WiFi service
    esp_wifi_start();
    // Print a message to indicate that WiFi initialization is complete
    ESP_LOGI(TAG, "wifi_init finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    esp_wifi_connect();
    ESP_LOGI(TAG, "Event group wait complete.");

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s", WIFI_SSID);
        wifi_connected = true;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", WIFI_SSID);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    vTaskDelete(NULL);
}

static void update_septic_tank(char *sensor_read){
    // Initialize buffer with zeros
    ESP_LOGI(TAG, "Septic tank level: %s", sensor_read);
    lv_label_set_text(ui_Septic_Level_Value, sensor_read);
    // Clear the buffer after use
    ESP_LOGI(TAG, "Clear character array: %s", sensor_read);
}

// MQTT Event Handler
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected to Home Assistant broker");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Disconnected from Home Assistant broker");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT Successfully subscribed to topic, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT Unsubscribed from topic, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT Data received:");
            ESP_LOGI(TAG, "Topic: %.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "Data: %.*s", event->data_len, event->data);
            
            int sensor_read = atoi(event->data);
            lv_arc_set_value(ui_Arc3, sensor_read);
            
            char data[8];
            snprintf(data, sizeof(data), "%d%%", sensor_read);
            update_septic_tank(data);
            memset(data, 0, sizeof(data));
    
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT Error event occurred");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
                ESP_LOGI(TAG, "Last captured errno : %d (%s)", event->error_handle->esp_transport_sock_errno,
                        strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGI(TAG, "Other MQTT event id: %d", event->event_id);
            break;
    }
}

esp_mqtt_client_handle_t initialize_mqtt(const char *broker_url, const char *client_id, 
                                       const char *username, const char *password)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = broker_url,
        .credentials.client_id = client_id,
        .credentials.username = username,
        .credentials.authentication.password = password,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    // if (client == NULL) {
        // ESP_LOGE(TAG, "Failed to initialize MQTT client");
        // return NULL;
    // }
    // Register MQTT events

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    // esp_mqtt_client_start(client);
    // Start MQTT client
    esp_err_t err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(err));
        esp_mqtt_client_destroy(client);
        return NULL;
    }
    return client;
}

bool subscribe_topic(esp_mqtt_client_handle_t client, const char *topic, int qos)
{
    if (client == NULL || topic == NULL) {
        ESP_LOGE(TAG, "Invalid client handle or topic");
        return false;
    }
    int msg_id = esp_mqtt_client_subscribe(client, topic, qos);
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Failed to subscribe to topic %s", topic);
        return false;
    }
    ESP_LOGI(TAG, "Sent subscription request for topic %s with msg_id=%d", topic, msg_id);
    return true;
}

bool publish_topic(esp_mqtt_client_handle_t client, const char *topic, const char *data, int qos)
{
    if (client == NULL || topic == NULL || data == NULL) {
        ESP_LOGE(TAG, "Invalid client handle, topic, or data");
        return false;
    }
    int msg_id = esp_mqtt_client_publish(client, topic, (int)strlen(data), data, qos, 0);
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Failed to publish data to topic %s", topic);
        return false;
    }
    ESP_LOGI(TAG, "Published data to topic %s with msg_id=%d", topic, msg_id);
    return true;
}

static void massage_switch_cb(lv_event_t *e) {
    static bool switch_state = false;
    switch_state = !switch_state;
    ESP_LOGI("massage switch", "click, current state: %s mode", switch_state ? "on" : "off");
}

static void bright_switch_cb(lv_event_t *e) {
    static bool switch_state = false;
    switch_state = !switch_state;
    ESP_LOGI("bright switch", "click, current state: %s mode", switch_state ? "on" : "off");
}

static void massage_switch_state(){
    lv_obj_t *switch_button = ui_Switch4;
    lv_obj_add_event_cb(switch_button, massage_switch_cb, LV_EVENT_CLICKED, NULL);
}

static void bright_switch_state(){
    lv_obj_t *switch_button = ui_Switch1;
    lv_obj_add_event_cb(switch_button, bright_switch_cb, LV_EVENT_CLICKED, NULL);
}

static void update_water_tank(int sensor_read){
    lv_arc_set_value(ui_Arc2, sensor_read);
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d%%", sensor_read);
    lv_label_set_text(ui_Water_Level_Value, buffer);
}



// LVGL tick task
static void lv_tick_task(void *arg) {
    while (1) {
        lv_tick_inc(portTICK_PERIOD_MS);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// FROM Claude:
void app_main(void) {
    const char *mqtt_broker = "mqtt://192.168.50.47";
    const char *client_id = "SenseCAP_D1";
    const char *username = "mqtt_indicator_1";
    const char *password = "mqtt";

    // // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize board and LVGL
    ESP_ERROR_CHECK(bsp_board_init());
    lv_port_init();
    xTaskCreate(wifi_init_task, "wifi_init_task", 4096, NULL, 5, NULL);
    xTaskCreate(lv_tick_task, "lv_tick_task", 4096, NULL, 1, NULL);

    // Initialize UI with semaphore protection
    lv_port_sem_take();
    ui_init();
    lv_port_sem_give();

    // // Set initial states
    int water_level1 = 50;
    int water_level2 = 50;
    update_water_tank(water_level2);
    bright_switch_state();
    massage_switch_state();
    // // Initialize client NULL
    esp_mqtt_client_handle_t client = NULL;
    ESP_LOGI(TAG, "Connected to WiFi");
    // If successfully created client return not NULL
    client = initialize_mqtt(mqtt_broker, client_id, username, password);

    while (1){
        esp_mqtt_client_subscribe(client, "water_level2", 1);
        int msg_id = esp_mqtt_client_subscribe(client, "water_level2", 1);
        if (msg_id == -1) {
            ESP_LOGE(TAG, "Failed to subscribe to topic");
        } else {
            ESP_LOGI(TAG, "Subscribed to topic, msg_id=%d", msg_id);
        }
        vTaskDelay(5000);
    }
}
