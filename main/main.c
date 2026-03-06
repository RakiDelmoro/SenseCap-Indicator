/**
 * SenseCAP Indicator - Home Assistant Controller
 * Main entry point
 */

#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "bsp_board.h"
#include "lv_port.h"
#include "ui.h"

#include "app_config.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "ui_handlers.h"

#define TAG "MAIN"

static void lv_tick_task(void *arg)
{
    while (1) {
        lv_tick_inc(portTICK_PERIOD_MS);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting SenseCAP Indicator Application");
    
    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    /* Initialize Board and LVGL */
    ESP_ERROR_CHECK(bsp_board_init());
    lv_port_init();
    
    /* Start LVGL tick task */
    xTaskCreate(lv_tick_task, "lv_tick", 2048, NULL, 1, NULL);
    
    /* Initialize UI */
    lv_port_sem_take();
    ui_init();
    ui_update_water_tank(50);  /* Set initial water tank level */
    lv_port_sem_give();
    
    /* Initialize WiFi */
    ESP_LOGI(TAG, "Initializing WiFi...");
    ESP_ERROR_CHECK(wifi_manager_init());
    
    /* Wait for WiFi connection */
    ret = wifi_manager_wait_for_ip(30000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi connection failed");
        return;
    }
    
    /* Initialize MQTT */
    ESP_LOGI(TAG, "Initializing MQTT...");
    ESP_ERROR_CHECK(mqtt_manager_init());
    
    /* Setup UI callbacks */
    ui_setup_callbacks(mqtt_manager_get_client());
    
    ESP_LOGI(TAG, "Application initialized successfully");
    
    /* Main loop */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
