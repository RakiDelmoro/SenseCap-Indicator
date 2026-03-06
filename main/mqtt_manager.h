#pragma once

#include "esp_err.h"
#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize MQTT manager
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_manager_init(void);

/**
 * @brief Publish data to MQTT topic
 * @param topic MQTT topic string
 * @param data Data to publish
 * @param qos Quality of Service level (0, 1, or 2)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_publish(const char *topic, const char *data, int qos);

/**
 * @brief Subscribe to MQTT topic
 * @param topic MQTT topic string
 * @param qos Quality of Service level (0, 1, or 2)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t mqtt_subscribe(const char *topic, int qos);

/**
 * @brief Get MQTT client handle
 * @return Client handle or NULL if not initialized
 */
esp_mqtt_client_handle_t mqtt_manager_get_client(void);

#ifdef __cplusplus
}
#endif
