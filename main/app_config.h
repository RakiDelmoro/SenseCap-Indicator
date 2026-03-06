#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* WiFi Configuration */
#define WIFI_SSID           ""
#define WIFI_PASSWORD       ""
#define WIFI_MAX_RETRY      5

/* MQTT Configuration */
#define MQTT_BROKER_URL     "mqtt://homeassistant.local"
#define MQTT_CLIENT_ID      ""
#define MQTT_USERNAME       ""
#define MQTT_PASSWORD       ""

/* MQTT Topics */
#define TOPIC_BRIGHT_STATE  "sensecap/bright_switch/state"
#define TOPIC_RELAX_STATE   "sensecap/relax_switch/state"
#define TOPIC_SENSORS       "esp/water-level"

#ifdef __cplusplus
}
#endif
