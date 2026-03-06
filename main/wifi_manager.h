#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize WiFi manager
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Wait for WiFi to obtain IP address
 * @param timeout_ms Maximum time to wait in milliseconds
 * @return ESP_OK if IP obtained, ESP_ERR_TIMEOUT if timeout
 */
esp_err_t wifi_manager_wait_for_ip(uint32_t timeout_ms);

/**
 * @brief Get WiFi connection status
 * @return true if connected with IP, false otherwise
 */
bool wifi_manager_is_connected(void);

#ifdef __cplusplus
}
#endif
