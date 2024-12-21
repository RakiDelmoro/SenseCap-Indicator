#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bsp_board.h"
#include "lv_port.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bsp_board.h"
#include "bsp_btn.h"


/*************  ✨ Codeium Command ⭐  *************/
#include "MQTTAsync.h"
#include "ui.h"

#define ADDRESS     "tcp://mqtt.eclipseprojects.io:1883"
#define CLIENTID    "ESPClient"
#define TOPIC       "esp/sensors"
#define QOS         1

static MQTTAsync client;

void messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    if (strcmp(topicName, TOPIC) == 0) {
        char *payload = (char *)message->payload;
        int sensor_value = atoi(payload);

        lv_port_sem_take();

        // Update UI elements
        _ui_arc_set_property(ui_Arc3, _UI_ARC_PROPERTY_VALUE, sensor_value);
        _ui_label_set_property(ui_Water_Level_Value, _UI_LABEL_PROPERTY_TEXT, payload);

        lv_port_sem_give();
    }

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
}

void mqtt_init(void) {
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTAsync_setCallbacks(client, NULL, NULL, messageArrived, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        ESP_LOGE("MQTT", "Failed to connect, return code %d", rc);
        return;
    }

    MQTTAsync_subscribe(client, TOPIC, QOS, NULL);
}

void app_main(void) {   
    ESP_ERROR_CHECK(bsp_board_init());
    lv_port_init();

    lv_port_sem_take();
    ui_init();
    lv_port_sem_give();

    mqtt_init();

    bsp_btn_register_callback(BOARD_BTN_ID_USER, BUTTON_SINGLE_CLICK, __button_click_callback, NULL);
}

