#pragma once

#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup UI event callbacks with MQTT client
 * @param mqtt_client MQTT client handle for publishing
 */
void ui_setup_callbacks(esp_mqtt_client_handle_t mqtt_client);

/**
 * @brief Toggle bright switch state and publish
 * @param mqtt_client MQTT client handle
 * @return true if new state is ON, false if OFF
 */
bool ui_bright_switch_toggle(esp_mqtt_client_handle_t mqtt_client);

/**
 * @brief Toggle relax switch state and publish
 * @param mqtt_client MQTT client handle
 * @return true if new state is ON, false if OFF
 */
bool ui_relax_switch_toggle(esp_mqtt_client_handle_t mqtt_client);

/**
 * @brief Update water tank display
 * @param percentage Water level percentage (0-100)
 */
void ui_update_water_tank(int percentage);

#ifdef __cplusplus
}
#endif
