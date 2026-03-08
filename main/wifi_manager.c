#include "wifi_manager.h"
#include "app_config.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip4_addr.h"

#define TAG "WIFI_MANAGER"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t s_wifi_event_group = NULL;
static int s_retry_num = 0;
static bool s_wifi_has_ip = false;  
static esp_netif_t *s_wifi_netif = NULL;
static esp_netif_t *s_sta_netif = NULL;

static void wifi_manager_restart(void)
{
    ESP_LOGI(TAG, "Restarting WiFi from scratch...");
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    s_wifi_has_ip = false;
    esp_wifi_stop();
    esp_wifi_start();
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi station starting, attempting connection...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* event = event_data;
        ESP_LOGW(TAG, "WiFi disconnected, reason: %d", event->reason);
        
        if (s_retry_num < WIFI_MAX_RETRY) {
            ESP_LOGI(TAG, "Retry attempt %d/%d", s_retry_num + 1, WIFI_MAX_RETRY);

            int delay_ms = 1000 * (1 << s_retry_num);
            if (delay_ms > 30000) delay_ms = 30000;
            
            ESP_LOGI(TAG, "Waiting %d ms before reconnect...", delay_ms);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
            s_retry_num++;
            esp_wifi_connect();
        } else {
            ESP_LOGE(TAG, "WiFi connection failed after %d attempts", WIFI_MAX_RETRY);
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = event_data;
        ESP_LOGI(TAG, "WiFi IP obtained: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        s_wifi_has_ip = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_manager_init(void)
{
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        return ESP_ERR_NO_MEM;
    }
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    s_wifi_netif = esp_netif_create_default_wifi_sta();

    if (s_wifi_netif != NULL) {
        esp_netif_ip_info_t ip_info;
        IP4_ADDR(&ip_info.ip, 000, 000, 00, 00);
        IP4_ADDR(&ip_info.gw, 000, 000, 00, 00);
        IP4_ADDR(&ip_info.netmask, 000, 000, 000, 0);

        ESP_ERROR_CHECK(esp_netif_dhcpc_stop(s_wifi_netif));
        ESP_ERROR_CHECK(esp_netif_set_ip_info(s_wifi_netif, &ip_info));
        ESP_LOGI(TAG, "Static IP configured successfully");
    }
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler, NULL, NULL));
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi manager initialized");
    
    /* Retry logic for initial connection */
    while (s_retry_num < WIFI_MAX_RETRY) {
        ESP_LOGI(TAG, "WiFi connection attempt %d/%d", s_retry_num + 1, WIFI_MAX_RETRY);
        
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                pdFALSE, pdFALSE,
                pdMS_TO_TICKS(30000));
        
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi connected successfully on attempt %d", s_retry_num + 1);
            return ESP_OK;
        }
        
        if (bits & WIFI_FAIL_BIT) {
            ESP_LOGE(TAG, "WiFi connection failed after all disconnect retries");
            return ESP_FAIL;
        }
        
        /* Timeout - wait with exponential backoff then restart */
        int delay_ms = 1000 * (1 << s_retry_num);
        if (delay_ms > 30000) delay_ms = 30000;
        ESP_LOGW(TAG, "WiFi connection timeout, waiting %d ms before restart...", delay_ms);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        s_retry_num++;
        wifi_manager_restart();
    }
    
    ESP_LOGE(TAG, "WiFi connection failed after %d attempts", WIFI_MAX_RETRY);
    return ESP_FAIL;
}

esp_err_t wifi_manager_wait_for_ip(uint32_t timeout_ms)
{
    if (s_wifi_event_group == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE, pdFALSE,
            pdMS_TO_TICKS(timeout_ms));
    
    if (bits & WIFI_CONNECTED_BIT) {
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        return ESP_FAIL;
    } else {
        return ESP_ERR_TIMEOUT;
    }
}

bool wifi_manager_is_connected(void)
{
    return s_wifi_has_ip;
}
