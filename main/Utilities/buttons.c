#include "indicator_btn.h"
#include "indicator_display.h"
#include "bsp_btn.h"
#include "esp_timer.h"

static uint32_t hold_cnt=0;
static bool sleep_flag=false;
static bool sleep_start_flag=false;

static esp_timer_handle_t   factory_reset_timer_handle;

static void __factory_reset_callback(void* arg)
{
    ESP_ERROR_CHECK(nvs_flash_erase());
    fflush(stdout);
    esp_restart();
}

static void __btn_click_callback(void* arg)
{
    bool st=0;
    if( sleep_flag ) {
        ESP_LOGI("btn", "click, cur st: sleep mode, restart!");
        fflush(stdout);
        esp_restart();
        return;
    }
    if( indicator_display_st_get()) {
        ESP_LOGI("btn", "click, off");
        indicator_display_off();

        st = 0;
        esp_event_post_to(view_event_handle, VIEW_EVENT_BASE, VIEW_EVENT_SCREEN_CTRL, &st, sizeof(st), portMAX_DELAY);
    } else {
        ESP_LOGI("btn", "click, on");
        
        st = 1;
        esp_event_post_to(view_event_handle, VIEW_EVENT_BASE, VIEW_EVENT_SCREEN_CTRL, &st, sizeof(st), portMAX_DELAY);

        indicator_display_on();
    }
}

int indicator_btn_init(void)
{
    bsp_btn_register_callback( BOARD_BTN_ID_USER, BUTTON_SINGLE_CLICK,     __btn_click_callback, NULL);
}
