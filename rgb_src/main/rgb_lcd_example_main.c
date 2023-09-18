/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#pragma GCC optimize("Ofast,unroll-loops")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch_ft5x06.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "ui.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "esp_sntp.h"

#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_http_client.h"

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#include "cJSON.h"

cJSON *root;

#define I2C_MASTER_SCL_IO 18
#define I2C_MASTER_SDA_IO 8
#define I2C_MASTER_NUM 0
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TX_BUF 0
#define I2C_MASTER_RX_BUF 0
#define I2C_MASTER_TIMEOUT_MS 1000

// #include "s220_12fps.c"
char wifi_ssid, wifi_password;
lv_obj_t *pwd_ta;

static const char *TAG = "example";

lv_obj_t *img;

lv_obj_t *log_textarea;

lv_obj_t *screen1;

lv_obj_t *state_line;
lv_obj_t *stste_line_bat_logo;
lv_obj_t *ssid_value;
lv_obj_t *bat_value;
lv_obj_t *dd;

lv_obj_t *obj1;
lv_obj_t *obj2;
lv_obj_t *obj3;
lv_obj_t *obj4;

lv_obj_t *large_obj;
lv_obj_t *large_obj1;

static lv_obj_t *chart1;
static lv_obj_t *chart2;
lv_chart_series_t *ser1;
lv_chart_series_t *ser2;

lv_obj_t *temp_value;
lv_obj_t *hume_value;
lv_obj_t *time_large_vaule;
lv_obj_t *time_min_vaule;
lv_obj_t *time_date_vaule;

lv_obj_t *weather_icon_day_1;
lv_obj_t *weather_icon_day_2;
lv_obj_t *weather_icon_day_3;
lv_obj_t *weather_text_day_1;
lv_obj_t *weather_text_day_2;
lv_obj_t *weather_text_day_3;
lv_obj_t *weather_temp_day_1;
lv_obj_t *weather_temp_day_2;
lv_obj_t *weather_temp_day_3;
lv_obj_t *weather_hume_day_1;
lv_obj_t *weather_hume_day_2;
lv_obj_t *weather_hume_day_3;
lv_obj_t *weather_hume_logo_day_1;
lv_obj_t *weather_hume_logo_day_2;
lv_obj_t *weather_hume_logo_day_3;
lv_obj_t *weather_windy_day_1;
lv_obj_t *weather_windy_day_2;
lv_obj_t *weather_windy_day_3;

lv_obj_t *meter;
lv_obj_t *mem_info;

lv_meter_scale_t *scale;
lv_meter_indicator_t *indic1;

lv_obj_t *screen_br;
lv_obj_t *evn_br;
lv_obj_t *screen_opa;
lv_obj_t *tv;
lv_obj_t *label_evn_br;
lv_obj_t *label_screen_br;

static lv_obj_t *kb;

static lv_anim_timeline_t *anim_timeline = NULL;

lv_anim_t evn_br_anim;

int opa_default = 220;

#define USR_SYMBOL_SUN_100 "\xEF\x84\x81"
#define USR_SYMBOL_CLOUDE_101 "\xEF\x84\x82"
#define USR_SYMBOL_SHADE_104 "\xEF\x84\x85"

#define USR_SYMBOL_117 "\xEF\x84\x92" // 小雨
#define USR_SYMBOL_119 "\xEF\x84\x94" // 大雨

#define USR_SYMBOL_RAIN_305 "\xEF\x84\x90"
#define USR_SYMBOL_SNOW_499 "\xEF\x84\xAD"
#define USR_SYMBOL_HUME_11F "\xEF\x84\x9F"
#define USR_SYMBOL_WIND_14C "\xEF\x85\x8C"

#define USR_SYMBOL_TEST_60 "\xEF\x84\x85"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_BK_LIGHT 44
#define EXAMPLE_PIN_NUM_HSYNC -1 // HSYNC
#define EXAMPLE_PIN_NUM_VSYNC -1 // VSYNC
#define EXAMPLE_PIN_NUM_DE 1
#define EXAMPLE_PIN_NUM_PCLK 46
#define EXAMPLE_PIN_NUM_DATA0 39  // B0
#define EXAMPLE_PIN_NUM_DATA1 40  // B1
#define EXAMPLE_PIN_NUM_DATA2 41  // B2
#define EXAMPLE_PIN_NUM_DATA3 42  // B3
#define EXAMPLE_PIN_NUM_DATA4 2   // B4
#define EXAMPLE_PIN_NUM_DATA5 14  // G0
#define EXAMPLE_PIN_NUM_DATA6 21  // G1
#define EXAMPLE_PIN_NUM_DATA7 47  // G2
#define EXAMPLE_PIN_NUM_DATA8 48  // G3
#define EXAMPLE_PIN_NUM_DATA9 45  // G4
#define EXAMPLE_PIN_NUM_DATA10 38 // G5
#define EXAMPLE_PIN_NUM_DATA11 9  // R0
#define EXAMPLE_PIN_NUM_DATA12 10 // R1
#define EXAMPLE_PIN_NUM_DATA13 11 // R2
#define EXAMPLE_PIN_NUM_DATA14 12 // R3
#define EXAMPLE_PIN_NUM_DATA15 13 // R4
#define EXAMPLE_PIN_NUM_DISP_EN -1

// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES 800
#define EXAMPLE_LCD_V_RES 480

#if CONFIG_EXAMPLE_DOUBLE_FB
#define EXAMPLE_LCD_NUM_FB 2
#else
#define EXAMPLE_LCD_NUM_FB 1
#endif // CONFIG_EXAMPLE_DOUBLE_FB

#define EXAMPLE_LVGL_TICK_PERIOD_MS 2

// we use two semaphores to sync the VSYNC event and the LVGL task, to avoid potential tearing effect
#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
SemaphoreHandle_t sem_vsync_end;
SemaphoreHandle_t sem_gui_ready;
#endif

extern void example_lvgl_demo_ui(lv_disp_t *disp);

static bool example_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data)
{
  BaseType_t high_task_awoken = pdFALSE;
#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
  if (xSemaphoreTakeFromISR(sem_gui_ready, &high_task_awoken) == pdTRUE)
  {
    xSemaphoreGiveFromISR(sem_vsync_end, &high_task_awoken);
  }
#endif
  return high_task_awoken == pdTRUE;
}

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
  int offsetx1 = area->x1;
  int offsetx2 = area->x2;
  int offsety1 = area->y1;
  int offsety2 = area->y2;
#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
  xSemaphoreGive(sem_gui_ready);
  xSemaphoreTake(sem_vsync_end, portMAX_DELAY);
#endif
  // pass the draw buffer to the driver
  esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
  lv_disp_flush_ready(drv);
}

static void example_increase_lvgl_tick(void *arg)
{
  /* Tell LVGL how many milliseconds has elapsed */
  lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

esp_lcd_touch_handle_t tp_handle = NULL;

static void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  esp_lcd_touch_read_data(tp_handle);

  uint16_t touch_x[1];
  uint16_t touch_y[1];
  uint16_t touch_strength[1];
  uint8_t touch_cnt = 0;

  bool touchpad_pressed = esp_lcd_touch_get_coordinates(tp_handle, touch_x, touch_y, touch_strength, &touch_cnt, 1);
  if (touchpad_pressed)
  {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touch_x[0];
    data->point.y = touch_y[0];
    ESP_LOGI(TAG, "x:%d, y:%d", touch_x[0], touch_y[0]);
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

static esp_err_t user_init_sdcard()
{
  esp_err_t ret;
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 100,
      .allocation_unit_size = 16 * 1024};
  sdmmc_card_t *card;
  const char mount_point[] = "/sdcard";
  ESP_LOGI(TAG, "Initializing SD card");

  // Use settings defined above to initialize SD card and mount FAT filesystem.
  // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
  // Please check its source code and implement error recovery when developing
  // production applications.

  ESP_LOGI(TAG, "Using SDMMC peripheral");
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();

  // This initializes the slot without card detect (CD) and write protect (WP) signals.
  // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

  slot_config.width = 4;

  // On chips where the GPIOs used for SD card can be configured, set them in
  // the slot_config structure:
  slot_config.clk = 3;
  slot_config.cmd = 0;
  slot_config.d0 = 5;
  slot_config.d1 = 4;
  slot_config.d2 = 7;
  slot_config.d3 = 6;

  ESP_LOGI(TAG, "Mounting filesystem");
  ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

  if (ret != ESP_OK)
  {
    if (ret == ESP_FAIL)
    {
      ESP_LOGE(TAG, "Failed to mount filesystem. "
                    "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
    }
    else
    {
      ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                    "Make sure SD card lines have pull-up resistors in place.",
               esp_err_to_name(ret));
    }
    return ret;
  }
  ESP_LOGI(TAG, "Filesystem mounted");

  // Card has been initialized, print its properties
  sdmmc_card_print_info(stdout, card);

  return ret;
}

// 枚举icon，unicode范围外需要添加宏定义，需要utf8硬编码
static void update_weather_icon(lv_obj_t *update_label, int icon_state)
{
  switch (icon_state)
  {
  case 13:
    lv_label_set_text(update_label, USR_SYMBOL_117);
    break;
  case 14:
    lv_label_set_text(update_label, USR_SYMBOL_119);
    break;
  case 100:
    lv_label_set_text(update_label, USR_SYMBOL_SUN_100);
    break;
  case 101:
    lv_label_set_text(update_label, USR_SYMBOL_CLOUDE_101);
    break;
  case 104:
    lv_label_set_text(update_label, USR_SYMBOL_SHADE_104);
    break;
  case 305:
    lv_label_set_text(update_label, USR_SYMBOL_RAIN_305);
    break;
  case 499:
    lv_label_set_text(update_label, USR_SYMBOL_SNOW_499);
    break;
  default:
    lv_label_set_text(update_label, USR_SYMBOL_WIND_14C);
    // Serial.println("don't define this icon");
  }
  // Serial.println(icon_state);
}

// 更新天气
void get_weather_forecast(lv_timer_t *timer)
{
  cJSON *cjson_item = cJSON_GetObjectItem(root, "results");
  cJSON *cjson_results = cJSON_GetArrayItem(cjson_item, 0);
  cJSON *cjson_daily = cJSON_GetObjectItem(cjson_results, "daily");
  cJSON *cjson_day0 = cJSON_GetArrayItem(cjson_daily, 0);
  cJSON *cjson_text = cJSON_GetObjectItem(cjson_day0, "text_day");
  char *tmp, *high, *low;
  tmp = cjson_text->valuestring;
  lv_label_set_text_fmt(weather_text_day_1, "今天%s", tmp);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 1);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "text_day");
  tmp = cjson_text->valuestring;
  lv_label_set_text_fmt(weather_text_day_2, "明天%s", tmp);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 2);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "text_day");
  tmp = cjson_text->valuestring;
  lv_label_set_text_fmt(weather_text_day_3, "后天%s", tmp);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 0);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "low");
  low = cjson_text->valuestring;
  cjson_text = cJSON_GetObjectItem(cjson_day0, "high");
  high = cjson_text->valuestring;
  lv_label_set_text_fmt(weather_temp_day_1, "#66CCFF %s℃#-#FF0000 %s℃#", low, high);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 1);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "low");
  low = cjson_text->valuestring;
  cjson_text = cJSON_GetObjectItem(cjson_day0, "high");
  high = cjson_text->valuestring;
  lv_label_set_text_fmt(weather_temp_day_2, "#66CCFF %s℃#-#FF0000 %s℃#", low, high);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 2);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "low");
  low = cjson_text->valuestring;
  cjson_text = cJSON_GetObjectItem(cjson_day0, "high");
  high = cjson_text->valuestring;
  lv_label_set_text_fmt(weather_temp_day_3, "#66CCFF %s℃#-#FF0000 %s℃#", low, high);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 0);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "humidity");
  tmp = cjson_text->valuestring;
  lv_label_set_text_fmt(weather_hume_day_1, "   %s", tmp);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 1);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "humidity");
  tmp = cjson_text->valuestring;
  lv_label_set_text_fmt(weather_hume_day_2, "   %s", tmp);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 2);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "humidity");
  tmp = cjson_text->valuestring;
  lv_label_set_text_fmt(weather_hume_day_3, "   %s", tmp);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 0);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "wind_scale");
  low = cjson_text->valuestring;
  cjson_text = cJSON_GetObjectItem(cjson_day0, "wind_direction");
  high = cjson_text->valuestring;
  lv_label_set_text_fmt(weather_windy_day_1, "%s级%s风", low, high);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 1);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "wind_scale");
  low = cjson_text->valuestring;
  cjson_text = cJSON_GetObjectItem(cjson_day0, "wind_direction");
  high = cjson_text->valuestring;
  lv_label_set_text_fmt(weather_windy_day_2, "%s级%s风", low, high);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 2);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "wind_scale");
  low = cjson_text->valuestring;
  cjson_text = cJSON_GetObjectItem(cjson_day0, "wind_direction");
  high = cjson_text->valuestring;
  lv_label_set_text_fmt(weather_windy_day_3, "%s级%s风", low, high);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 0);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "code_day");
  high = cjson_text->valuestring;
  lv_textarea_add_text(log_textarea, high);
  lv_textarea_add_char(log_textarea, '\n');
  int intNum = atoi(high);
  update_weather_icon(weather_icon_day_1, intNum);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 1);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "code_day");
  high = cjson_text->valuestring;
  lv_textarea_add_text(log_textarea, high);
  lv_textarea_add_char(log_textarea, '\n');
  intNum = atoi(high);
  update_weather_icon(weather_icon_day_2, intNum);

  cjson_day0 = cJSON_GetArrayItem(cjson_daily, 2);
  cjson_text = cJSON_GetObjectItem(cjson_day0, "code_day");
  high = cjson_text->valuestring;
  lv_textarea_add_text(log_textarea, high);
  lv_textarea_add_char(log_textarea, '\n');
  intNum = atoi(high);
  update_weather_icon(weather_icon_day_3, intNum);
}

// 更新时间
void get_time_value(lv_timer_t *timer)
{

  struct tm timeinfo;
  time_t now = 0;
  time(&now);
  localtime_r(&now, &timeinfo);

  char timeHour[6];
  strftime(timeHour, 6, "%H:%M", &timeinfo);

  char timeSec[3];
  strftime(timeSec, 3, "%S", &timeinfo);

  char timeDate[6];
  strftime(timeDate, 6, "%m-%d", &timeinfo);

  lv_label_set_text_fmt(time_large_vaule, "%s", timeHour);
  lv_label_set_text_fmt(time_min_vaule, "%s", timeSec);
  lv_label_set_text_fmt(time_date_vaule, "%s", timeDate);
}

static void set_angle(void *obj, int32_t v)
{
  lv_arc_set_value((lv_obj_t *)obj, v);
}

static void set_br(void *bar, int32_t temp)
{
  lv_bar_set_value((lv_obj_t *)bar, temp, LV_ANIM_ON);
}

void anim_x_cb(void *var, int32_t v)
{
  lv_obj_set_x((lv_obj_t *)var, v);
}

static void slider_event_cb(lv_event_t *e)
{
  // ledcWrite(LEDC_CHANNEL_0, (int)lv_slider_get_value(screen_br));
  lv_label_set_text_fmt(label_screen_br, "背光亮度:\n  %d%%", 0);
}

static void opa_cb(lv_event_t *e)
{
  // printf("%d\n", (int)lv_slider_get_value(screen_opa));
  opa_default = (int)lv_slider_get_value(screen_opa);
  lv_obj_set_style_opa(tv, opa_default, 0);
  lv_obj_set_style_opa(log_textarea, opa_default, 0);
}

static void event_dd(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    char buf[32];
    lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
    /// LV_LOG_USER("Option: %s", buf);
  }
}

static void ta_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED)
  {
    /*Focus on the clicked text area*/
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    if (kb != NULL)
      lv_keyboard_set_textarea(kb, ta);
  }

  else if (code == LV_EVENT_READY)
  {
    LV_LOG_USER("Ready, current text: %s", lv_textarea_get_text(ta));
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  }
}

void anim_main_ui()
{
  lv_anim_t anim_obj1;
  lv_anim_init(&anim_obj1);
  lv_anim_set_var(&anim_obj1, obj1);
  lv_anim_set_values(&anim_obj1, lv_obj_get_x(obj1) - 350, lv_obj_get_x(obj1) - 7);
  lv_anim_set_time(&anim_obj1, 2000);
  lv_anim_set_exec_cb(&anim_obj1, anim_x_cb);
  lv_anim_set_path_cb(&anim_obj1, lv_anim_path_linear);
  // lv_anim_start(&anim_obj1);

  lv_anim_t anim_state_line;
  lv_anim_init(&anim_state_line);
  lv_anim_set_var(&anim_state_line, state_line);
  lv_anim_set_values(&anim_state_line, lv_obj_get_x(state_line) + 350, lv_obj_get_x(state_line) + 7);
  lv_anim_set_time(&anim_state_line, 2000);
  lv_anim_set_exec_cb(&anim_state_line, anim_x_cb);
  lv_anim_set_path_cb(&anim_state_line, lv_anim_path_linear);
  // lv_anim_start(&anim_state_line);

  lv_anim_t anim_obj2;
  lv_anim_init(&anim_obj2);
  lv_anim_set_var(&anim_obj2, obj2);
  lv_anim_set_values(&anim_obj2, lv_obj_get_x(obj2) + 350, lv_obj_get_x(obj2) + 7);
  lv_anim_set_time(&anim_obj2, 2000);
  lv_anim_set_exec_cb(&anim_obj2, anim_x_cb);
  lv_anim_set_path_cb(&anim_obj2, lv_anim_path_linear);
  // lv_anim_start(&anim_obj2);

  lv_anim_t anim_large_obj;
  lv_anim_init(&anim_large_obj);
  lv_anim_set_var(&anim_large_obj, large_obj);
  lv_anim_set_values(&anim_large_obj, lv_obj_get_x(large_obj) - 500, lv_obj_get_x(large_obj));
  lv_anim_set_time(&anim_large_obj, 2000);
  lv_anim_set_exec_cb(&anim_large_obj, anim_x_cb);
  lv_anim_set_path_cb(&anim_large_obj, lv_anim_path_linear);
  // lv_anim_start(&anim_large_obj);

  lv_anim_t anim_obj3;
  lv_anim_init(&anim_obj3);
  lv_anim_set_var(&anim_obj3, obj3);
  lv_anim_set_values(&anim_obj3, lv_obj_get_x(obj3) - 350, lv_obj_get_x(obj3) - 7);
  lv_anim_set_time(&anim_obj3, 2000);
  lv_anim_set_exec_cb(&anim_obj3, anim_x_cb);
  lv_anim_set_path_cb(&anim_obj3, lv_anim_path_linear);
  // lv_anim_start(&anim_obj3);

  lv_anim_t anim_obj4;
  lv_anim_init(&anim_obj4);
  lv_anim_set_var(&anim_obj4, obj4);
  lv_anim_set_values(&anim_obj4, lv_obj_get_x(obj4) + 350, lv_obj_get_x(obj4) + 7);
  lv_anim_set_time(&anim_obj4, 2000);
  lv_anim_set_exec_cb(&anim_obj4, anim_x_cb);
  lv_anim_set_path_cb(&anim_obj4, lv_anim_path_linear);
  // lv_anim_start(&anim_obj4);

  lv_anim_t anim_large_obj1;
  lv_anim_init(&anim_large_obj1);
  lv_anim_set_var(&anim_large_obj1, large_obj1);
  lv_anim_set_values(&anim_large_obj1, lv_obj_get_x(large_obj1) + 500, lv_obj_get_x(large_obj1));
  lv_anim_set_time(&anim_large_obj1, 2000);
  lv_anim_set_exec_cb(&anim_large_obj1, anim_x_cb);
  lv_anim_set_path_cb(&anim_large_obj1, lv_anim_path_linear);
  // lv_anim_start(&anim_large_obj1);

  anim_timeline = lv_anim_timeline_create();
  lv_anim_timeline_add(anim_timeline, 0, &anim_obj1);
  lv_anim_timeline_add(anim_timeline, 300, &anim_state_line);
  lv_anim_timeline_add(anim_timeline, 300, &anim_obj2);

  lv_anim_timeline_add(anim_timeline, 600, &anim_large_obj);

  lv_anim_timeline_add(anim_timeline, 900, &anim_obj3);
  lv_anim_timeline_add(anim_timeline, 900, &anim_obj4);

  lv_anim_timeline_add(anim_timeline, 1200, &anim_large_obj1);
}

void wifi_scan()
{

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  uint16_t number = 10;
  wifi_ap_record_t ap_info[10];
  uint16_t ap_count = 0;
  memset(ap_info, 0, sizeof(ap_info));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
  esp_wifi_scan_start(NULL, true);
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

  for (int i = 0; (i < 10) && (i < ap_count); i++)
  {

    ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
    const char *ssid = (const char *)ap_info[i].ssid;
    lv_dropdown_add_option(dd, ssid, LV_DROPDOWN_POS_LAST);
  }

  lv_textarea_add_text(log_textarea, "完成");
  lv_textarea_add_char(log_textarea, '\n'); // 换行
}

// 主界面创建
void main_ui()
{
  LV_FONT_DECLARE(xiaolai_20);
  LV_FONT_DECLARE(weather_28);
  LV_FONT_DECLARE(xiaolai_48);
  LV_FONT_DECLARE(weather_60);
  LV_FONT_DECLARE(xiaolai_70);

  LV_IMG_DECLARE(background);

  lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);

  screen1 = lv_obj_create(img);
  lv_obj_set_size(screen1, 480, 480);
  lv_obj_align(screen1, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_opa(screen1, 0, 0);
  lv_obj_set_style_border_opa(screen1, 0, 0);
  lv_obj_set_scrollbar_mode(screen1, LV_SCROLLBAR_MODE_OFF);
  lv_obj_add_flag(screen1, LV_OBJ_FLAG_SCROLL_MOMENTUM);
  // lv_obj_set_style_opa(screen1, 0, 0);
  // lv_obj_clear_flag(screen1, LV_OBJ_FLAG_SCROLLABLE);

  obj1 = lv_obj_create(screen1);
  lv_obj_set_size(obj1, 160, 200);
  lv_obj_align(obj1, LV_ALIGN_TOP_LEFT, -5, 0);
  lv_obj_set_style_opa(obj1, opa_default, 0);
  lv_obj_clear_flag(obj1, LV_OBJ_FLAG_SCROLLABLE);

  obj2 = lv_obj_create(screen1);
  lv_obj_set_size(obj2, 290, 160);
  lv_obj_align(obj2, LV_ALIGN_TOP_RIGHT, 10, 40);
  lv_obj_set_style_opa(obj2, opa_default, 0);
  lv_obj_clear_flag(obj2, LV_OBJ_FLAG_SCROLLABLE);

  state_line = lv_obj_create(screen1);
  lv_obj_set_size(state_line, 290, 30);
  lv_obj_align(state_line, LV_ALIGN_TOP_RIGHT, 10, 0);
  lv_obj_set_style_opa(state_line, opa_default, 0);
  lv_obj_clear_flag(state_line, LV_OBJ_FLAG_SCROLLABLE);

  time_large_vaule = lv_label_create(obj2);
  lv_label_set_long_mode(time_large_vaule, LV_LABEL_LONG_WRAP);
  lv_obj_align(time_large_vaule, LV_ALIGN_TOP_LEFT, -5, -5);
  lv_obj_set_style_text_font(time_large_vaule, &xiaolai_70, 0);
  lv_label_set_text_fmt(time_large_vaule, "%02d:%02d", 0, 0);
  lv_obj_set_style_text_color(time_large_vaule, lv_color_hex(0x9999ff), 0);
  lv_obj_set_style_bg_opa(time_large_vaule, 0, 0);
  lv_obj_set_style_text_opa(time_large_vaule, 255, 0);

  time_min_vaule = lv_label_create(obj2);
  lv_label_set_long_mode(time_min_vaule, LV_LABEL_LONG_WRAP);
  lv_obj_align(time_min_vaule, LV_ALIGN_TOP_RIGHT, 0, -5);
  lv_obj_set_style_text_font(time_min_vaule, &xiaolai_48, 0);
  lv_label_set_text_fmt(time_min_vaule, "%02d", 0);
  // lv_obj_set_style_text_color(time_min_vaule, lv_color_hex(0x9999ff), 0);
  lv_obj_set_style_bg_opa(time_min_vaule, 0, 0);
  lv_obj_set_style_text_opa(time_min_vaule, 255, 0);

  /*

  lv_obj_t *time_sec_vaule;
  time_sec_vaule = lv_label_create(obj2);
  lv_label_set_long_mode(time_sec_vaule, LV_LABEL_LONG_WRAP);
  lv_obj_align(time_sec_vaule, LV_ALIGN_TOP_MID, 10, 0);
  lv_obj_set_style_text_font(time_sec_vaule, &xiaolai_20, 0);
  lv_label_set_text_fmt(time_sec_vaule, "秒", 0, 0);
  // lv_obj_set_style_text_color(time_sec_vaule, lv_color_hex(0x9999ff), 0);
  lv_obj_set_style_bg_opa(time_sec_vaule, 0, 0);
  lv_obj_set_style_text_opa(time_sec_vaule, 255, 0);


    lv_obj_t *time_date;

    time_date = lv_label_create(obj2);
    lv_label_set_long_mode(time_date, LV_LABEL_LONG_WRAP);
    lv_obj_align(time_date, LV_ALIGN_LEFT_MID, -10, 28);
    lv_obj_set_style_text_font(time_date, &xiaolai_20, 0);
    lv_label_set_text_fmt(time_date, "日期:", 0, 0);
    lv_obj_set_style_text_color(time_date, lv_palette_main(LV_PALETTE_LIGHT_GREEN), 0);
    lv_obj_set_style_bg_opa(time_date, 0, 0);
    lv_obj_set_style_text_opa(time_date, 255, 0); */

  time_date_vaule = lv_label_create(obj2);
  lv_label_set_long_mode(time_date_vaule, LV_LABEL_LONG_WRAP);
  lv_obj_align(time_date_vaule, LV_ALIGN_BOTTOM_RIGHT, 5, 5);
  lv_obj_set_style_text_font(time_date_vaule, &xiaolai_48, 0);
  lv_label_set_text_fmt(time_date_vaule, "%d.%d", 8, 26);
  // lv_obj_set_style_text_color(time_min_vaule, lv_color_hex(0x9999ff), 0);
  lv_obj_set_style_bg_opa(time_date_vaule, 0, 0);
  lv_obj_set_style_text_opa(time_date_vaule, 255, 0);

  bat_value = lv_label_create(state_line);
  lv_label_set_long_mode(bat_value, LV_LABEL_LONG_WRAP);
  lv_obj_align(bat_value, LV_ALIGN_RIGHT_MID, 0, 0);
  // lv_obj_set_style_text_font(bat_value, &xiaolai_20, 0);
  lv_label_set_text_fmt(bat_value, "%d", 100);

  large_obj = lv_obj_create(screen1);
  lv_obj_set_size(large_obj, 460, 220);
  lv_obj_align(large_obj, LV_ALIGN_TOP_MID, 0, 210);
  lv_obj_set_style_opa(large_obj, opa_default, 0);
  lv_obj_clear_flag(large_obj, LV_OBJ_FLAG_SCROLLABLE);

  weather_icon_day_1 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_icon_day_1, LV_LABEL_LONG_WRAP);
  lv_obj_align(weather_icon_day_1, LV_ALIGN_TOP_LEFT, 35, -5);
  lv_obj_set_style_text_font(weather_icon_day_1, &weather_60, 0);
  lv_label_set_text_fmt(weather_icon_day_1, USR_SYMBOL_TEST_60);
  // lv_obj_set_style_text_color(weather_icon_day_1, lv_color_hex(0x9999ff), 0);
  lv_obj_set_style_bg_opa(weather_icon_day_1, 0, 0);
  lv_obj_set_style_text_opa(weather_icon_day_1, 255, 0);

  weather_icon_day_2 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_icon_day_2, LV_LABEL_LONG_WRAP);
  lv_obj_align(weather_icon_day_2, LV_ALIGN_TOP_MID, 0, -5);
  lv_obj_set_style_text_font(weather_icon_day_2, &weather_60, 0);
  lv_label_set_text_fmt(weather_icon_day_2, USR_SYMBOL_TEST_60);
  // lv_obj_set_style_text_color(weather_icon_day_2, lv_color_hex(0x9999ff), 0);
  lv_obj_set_style_bg_opa(weather_icon_day_2, 0, 0);
  lv_obj_set_style_text_opa(weather_icon_day_2, 255, 0);

  weather_icon_day_3 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_icon_day_3, LV_LABEL_LONG_WRAP);
  lv_obj_align(weather_icon_day_3, LV_ALIGN_TOP_RIGHT, -45, -5);
  lv_obj_set_style_text_font(weather_icon_day_3, &weather_60, 0);
  // lv_label_set_text_fmt(weather_icon_day_3, USR_SYMBOL_TEST_60);
  update_weather_icon(weather_icon_day_3, 100);
  // lv_obj_set_style_text_color(weather_icon_day_3, lv_color_hex(0x9999ff), 0);
  lv_obj_set_style_bg_opa(weather_icon_day_3, 0, 0);
  lv_obj_set_style_text_opa(weather_icon_day_3, 255, 0);

  weather_text_day_1 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_text_day_1, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_text_day_1, 150, 22);
  lv_obj_align(weather_text_day_1, LV_ALIGN_LEFT_MID, -10, -20);
  lv_obj_set_style_text_font(weather_text_day_1, &xiaolai_20, 0);
  lv_label_set_text(weather_text_day_1, "今天阴");
  lv_obj_set_style_text_align(weather_text_day_1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(weather_text_day_1, lv_color_hex(0x66CCFF), 0);
  lv_obj_set_style_bg_opa(weather_text_day_1, 0, 0);
  lv_obj_set_style_text_opa(weather_text_day_1, 255, 0);

  weather_text_day_2 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_text_day_2, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_text_day_2, 150, 22);
  lv_obj_align(weather_text_day_2, LV_ALIGN_CENTER, 0, -20);
  lv_obj_set_style_text_font(weather_text_day_2, &xiaolai_20, 0);
  lv_label_set_text(weather_text_day_2, "明天阴");
  lv_obj_set_style_text_align(weather_text_day_2, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(weather_text_day_2, lv_color_hex(0x66CCFF), 0);
  lv_obj_set_style_bg_opa(weather_text_day_2, 0, 0);
  lv_obj_set_style_text_opa(weather_text_day_2, 255, 0);

  weather_text_day_3 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_text_day_3, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_text_day_3, 150, 22);
  lv_obj_align(weather_text_day_3, LV_ALIGN_RIGHT_MID, 0, -20);
  lv_obj_set_style_text_font(weather_text_day_3, &xiaolai_20, 0);
  lv_label_set_text(weather_text_day_3, "后天阴");
  lv_obj_set_style_text_align(weather_text_day_3, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(weather_text_day_3, lv_color_hex(0x66CCFF), 0);
  lv_obj_set_style_bg_opa(weather_text_day_3, 0, 0);
  lv_obj_set_style_text_opa(weather_text_day_3, 255, 0);

  weather_temp_day_1 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_temp_day_1, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_temp_day_1, 150, 30);
  lv_obj_align(weather_temp_day_1, LV_ALIGN_LEFT_MID, -10, 10);
  lv_obj_set_style_text_font(weather_temp_day_1, &weather_28, 0);
  lv_label_set_recolor(weather_temp_day_1, true);
  lv_label_set_text_fmt(weather_temp_day_1, "#66CCFF %d℃#-#FF0000 %d℃#", 1, 5);
  lv_obj_set_style_text_align(weather_temp_day_1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_bg_opa(weather_temp_day_1, 0, 0);
  lv_obj_set_style_text_opa(weather_temp_day_1, 255, 0);

  weather_temp_day_2 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_temp_day_2, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_temp_day_2, 150, 30);
  lv_obj_align(weather_temp_day_2, LV_ALIGN_CENTER, 0, 10);
  lv_obj_set_style_text_font(weather_temp_day_2, &weather_28, 0);
  lv_label_set_recolor(weather_temp_day_2, true);
  lv_label_set_text_fmt(weather_temp_day_2, "#66CCFF %d℃#-#FF0000 %d℃#", 1, 5);
  lv_obj_set_style_text_align(weather_temp_day_2, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(weather_temp_day_2, lv_color_hex(0x66CCFF), 0);
  lv_obj_set_style_bg_opa(weather_temp_day_2, 0, 0);
  lv_obj_set_style_text_opa(weather_temp_day_2, 255, 0);

  weather_temp_day_3 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_temp_day_3, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_temp_day_3, 150, 30);
  lv_obj_align(weather_temp_day_3, LV_ALIGN_RIGHT_MID, 10, 10);
  lv_obj_set_style_text_font(weather_temp_day_3, &weather_28, 0);
  lv_label_set_recolor(weather_temp_day_3, true);
  lv_label_set_text_fmt(weather_temp_day_3, "#66CCFF %d℃#-#FF0000 %d℃#", 1, 5);
  lv_obj_set_style_text_align(weather_temp_day_3, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(weather_temp_day_3, lv_color_hex(0x66CCFF), 0);
  lv_obj_set_style_bg_opa(weather_temp_day_3, 0, 0);
  lv_obj_set_style_text_opa(weather_temp_day_3, 255, 0);

  weather_hume_logo_day_1 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_hume_logo_day_1, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_hume_logo_day_1, 100, 30);
  lv_obj_align(weather_hume_logo_day_1, LV_ALIGN_LEFT_MID, -10, 50);
  lv_obj_set_style_text_font(weather_hume_logo_day_1, &weather_28, 0);
  lv_label_set_recolor(weather_hume_logo_day_1, true);
  lv_label_set_text_static(weather_hume_logo_day_1, USR_SYMBOL_HUME_11F);
  lv_obj_set_style_text_align(weather_hume_logo_day_1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_bg_opa(weather_hume_logo_day_1, 0, 0);
  lv_obj_set_style_text_opa(weather_hume_logo_day_1, 255, 0);

  weather_hume_logo_day_2 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_hume_logo_day_2, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_hume_logo_day_2, 100, 30);
  lv_obj_align(weather_hume_logo_day_2, LV_ALIGN_CENTER, -25, 50);
  lv_obj_set_style_text_font(weather_hume_logo_day_2, &weather_28, 0);
  lv_label_set_recolor(weather_hume_logo_day_2, true);
  lv_label_set_text_static(weather_hume_logo_day_2, USR_SYMBOL_HUME_11F);
  lv_obj_set_style_text_align(weather_hume_logo_day_2, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_bg_opa(weather_hume_logo_day_2, 0, 0);
  lv_obj_set_style_text_opa(weather_hume_logo_day_2, 255, 0);

  weather_hume_logo_day_3 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_hume_logo_day_3, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_hume_logo_day_3, 100, 30);
  lv_obj_align(weather_hume_logo_day_3, LV_ALIGN_RIGHT_MID, -40, 50);
  lv_obj_set_style_text_font(weather_hume_logo_day_3, &weather_28, 0);
  lv_label_set_recolor(weather_hume_logo_day_3, true);
  lv_label_set_text_static(weather_hume_logo_day_3, USR_SYMBOL_HUME_11F);
  lv_obj_set_style_text_align(weather_hume_logo_day_3, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_bg_opa(weather_hume_logo_day_3, 0, 0);
  lv_obj_set_style_text_opa(weather_hume_logo_day_3, 255, 0);

  weather_hume_day_1 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_hume_day_1, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_hume_day_1, 150, 30);
  lv_obj_align(weather_hume_day_1, LV_ALIGN_LEFT_MID, -10, 45);
  lv_obj_set_style_text_font(weather_hume_day_1, &weather_28, 0);
  lv_label_set_recolor(weather_hume_day_1, true);
  lv_label_set_text_fmt(weather_hume_day_1, "   %d%%", 77);
  lv_obj_set_style_text_align(weather_hume_day_1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_bg_opa(weather_hume_day_1, 0, 0);
  lv_obj_set_style_text_opa(weather_hume_day_1, 255, 0);

  weather_hume_day_2 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_hume_day_2, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_hume_day_2, 150, 30);
  lv_obj_align(weather_hume_day_2, LV_ALIGN_CENTER, 0, 45);
  lv_obj_set_style_text_font(weather_hume_day_2, &weather_28, 0);
  lv_label_set_recolor(weather_hume_day_2, true);
  lv_label_set_text_fmt(weather_hume_day_2, "   %d%%", 77);
  lv_obj_set_style_text_align(weather_hume_day_2, LV_TEXT_ALIGN_CENTER, 0);
  // lv_obj_set_style_text_color(weather_hume_day_2, lv_color_hex(0x66CCFF), 0);
  lv_obj_set_style_bg_opa(weather_hume_day_2, 0, 0);
  lv_obj_set_style_text_opa(weather_hume_day_2, 255, 0);

  weather_hume_day_3 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_hume_day_3, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_hume_day_3, 150, 30);
  lv_obj_align(weather_hume_day_3, LV_ALIGN_RIGHT_MID, 10, 45);
  lv_obj_set_style_text_font(weather_hume_day_3, &weather_28, 0);
  lv_label_set_recolor(weather_hume_day_3, true);
  lv_label_set_text_fmt(weather_hume_day_3, "   %d%%", 77);
  lv_obj_set_style_text_align(weather_hume_day_3, LV_TEXT_ALIGN_CENTER, 0);
  // lv_obj_set_style_text_color(weather_hume_day_3, lv_color_hex(0x66CCFF), 0);
  lv_obj_set_style_bg_opa(weather_temp_day_3, 0, 0);
  lv_obj_set_style_text_opa(weather_hume_day_3, 255, 0);

  weather_windy_day_1 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_windy_day_1, LV_LABEL_LONG_WRAP);
  lv_obj_align(weather_windy_day_1, LV_ALIGN_LEFT_MID, -10, 80);
  lv_obj_set_size(weather_windy_day_1, 150, 22);
  lv_obj_set_style_text_font(weather_windy_day_1, &xiaolai_20, 0);
  lv_label_set_text_fmt(weather_windy_day_1, "%s级%s", "3-4", "东风");
  lv_obj_set_style_text_align(weather_windy_day_1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(weather_windy_day_1, lv_color_hex(0x66CCFF), 0);
  lv_obj_set_style_bg_opa(weather_windy_day_1, 0, 0);
  lv_obj_set_style_text_opa(weather_windy_day_1, 255, 0);

  weather_windy_day_2 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_windy_day_2, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_windy_day_2, 150, 22);
  lv_obj_align(weather_windy_day_2, LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_text_font(weather_windy_day_2, &xiaolai_20, 0);
  lv_label_set_text(weather_windy_day_2, "明天阴");
  lv_obj_set_style_text_align(weather_windy_day_2, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(weather_windy_day_2, lv_color_hex(0x66CCFF), 0);
  lv_obj_set_style_bg_opa(weather_windy_day_2, 0, 0);
  lv_obj_set_style_text_opa(weather_windy_day_2, 255, 0);

  weather_windy_day_3 = lv_label_create(large_obj);
  lv_label_set_long_mode(weather_windy_day_3, LV_LABEL_LONG_WRAP);
  lv_obj_set_size(weather_windy_day_3, 150, 22);
  lv_obj_align(weather_windy_day_3, LV_ALIGN_RIGHT_MID, 0, 80);
  lv_obj_set_style_text_font(weather_windy_day_3, &xiaolai_20, 0);
  lv_label_set_text(weather_windy_day_3, "后天阴");
  lv_obj_set_style_text_align(weather_windy_day_3, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(weather_windy_day_3, lv_color_hex(0x66CCFF), 0);
  lv_obj_set_style_bg_opa(weather_windy_day_3, 0, 0);
  lv_obj_set_style_text_opa(weather_windy_day_3, 255, 0);

  /*   chart1 = lv_chart_create(obj2);
    lv_obj_set_size(chart1, 180, 60);
    lv_obj_align(chart1, LV_ALIGN_TOP_RIGHT, 20, -15);
    lv_chart_set_range(chart1, LV_CHART_AXIS_PRIMARY_Y, -5, 30);
    lv_chart_set_type(chart1, LV_CHART_TYPE_BAR);
    lv_chart_set_update_mode(chart1, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_point_count(chart1, 12);

    ser1 = lv_chart_add_series(chart1, lv_palette_main(LV_PALETTE_DEEP_ORANGE), LV_CHART_AXIS_PRIMARY_Y);
    lv_obj_set_style_bg_opa(chart1, 0, 0);
    lv_obj_set_style_line_opa(chart1, 0, 0);
    lv_obj_set_style_border_opa(chart1, 0, 0); */

  lv_obj_t *temp;
  temp = lv_label_create(obj1);
  lv_label_set_long_mode(temp, LV_LABEL_LONG_WRAP);
  lv_obj_align(temp, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_set_style_text_font(temp, &xiaolai_20, 0);
  lv_label_set_text(temp, "温度");
  // lv_obj_set_style_text_color(temp, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(temp, 0, 0);
  lv_obj_set_style_text_opa(temp, 255, 0);

  temp_value = lv_label_create(obj1);
  lv_label_set_long_mode(temp_value, LV_LABEL_LONG_WRAP);
  lv_obj_align(temp_value, LV_ALIGN_TOP_MID, 0, 30);
  lv_obj_set_style_text_font(temp_value, &xiaolai_48, 0);
  lv_label_set_text_fmt(temp_value, "%.1f℃", 25.8);
  lv_obj_set_style_text_color(temp_value, lv_palette_main(LV_PALETTE_LIGHT_BLUE), 0);
  lv_obj_set_style_bg_opa(temp_value, 0, 0);

  /*
    chart2 = lv_chart_create(obj2);
    lv_obj_set_size(chart2, 180, 60);
    lv_obj_align(chart2, LV_ALIGN_TOP_RIGHT, 20, 60);
    lv_chart_set_range(chart2, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_type(chart2, LV_CHART_TYPE_BAR);
    lv_chart_set_update_mode(chart2, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_point_count(chart2, 12);

    ser2 = lv_chart_add_series(chart2, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    lv_obj_set_style_bg_opa(chart2, 0, 0);
    lv_obj_set_style_line_opa(chart2, 0, 0);
    lv_obj_set_style_border_opa(chart2, 0, 0);
  */

  lv_obj_t *hume;
  hume = lv_label_create(obj1);
  lv_label_set_long_mode(hume, LV_LABEL_LONG_WRAP);
  lv_obj_align(hume, LV_ALIGN_TOP_LEFT, 0, 85);
  lv_obj_set_style_text_font(hume, &xiaolai_20, 0);
  lv_label_set_text(hume, "湿度");
  // lv_obj_set_style_text_color(hume, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(hume, 0, 0);
  lv_obj_set_style_text_opa(hume, 255, 0);

  hume_value = lv_label_create(obj1);
  lv_label_set_long_mode(hume_value, LV_LABEL_LONG_WRAP);
  lv_obj_align(hume_value, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_text_font(hume_value, &xiaolai_48, 0);
  lv_label_set_text_fmt(hume_value, "%.1f%%", 50.2);
  lv_obj_set_style_text_color(hume_value, lv_palette_main(LV_PALETTE_DEEP_ORANGE), 0);
  lv_obj_set_style_bg_opa(hume_value, 0, 0);

  /*   for (int i = 0; i < 12; i++)
    {
      lv_chart_set_next_value(chart1, ser1, rand() % 30);
      lv_chart_set_next_value(chart2, ser2, rand() % 100);
    } */

  obj3 = lv_obj_create(screen1);
  lv_obj_set_size(obj3, 250, 200);
  lv_obj_align(obj3, LV_ALIGN_TOP_LEFT, -8, 440);
  lv_obj_set_style_opa(obj3, opa_default, 0);

  obj4 = lv_obj_create(screen1);
  lv_obj_set_size(obj4, 200, 200);
  lv_obj_align(obj4, LV_ALIGN_TOP_RIGHT, 8, 440);
  lv_obj_set_style_opa(obj4, opa_default, 0);
  lv_obj_clear_flag(obj4, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *cpu_info;
  cpu_info = lv_label_create(obj4);
  lv_label_set_long_mode(cpu_info, LV_LABEL_LONG_WRAP);
  lv_obj_align(cpu_info, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_text_font(cpu_info, &lv_font_montserrat_20, 0);
  lv_label_set_text(cpu_info, LV_SYMBOL_SETTINGS " ESP32-S3 Rev 0");
  // lv_obj_set_style_text_color(hume, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(cpu_info, 0, 0);
  lv_obj_set_style_text_opa(cpu_info, 255, 0);

  lv_obj_t *arc_men = lv_arc_create(obj4);
  lv_obj_set_size(arc_men, 100, 100);
  lv_arc_set_rotation(arc_men, 135);
  lv_arc_set_bg_angles(arc_men, 0, 270);
  lv_obj_align(arc_men, LV_ALIGN_CENTER, 0, 0);
  lv_obj_remove_style(arc_men, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(arc_men, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_opa(arc_men, 255, 0);

  lv_obj_t *men_info;
  men_info = lv_label_create(arc_men);
  lv_label_set_long_mode(men_info, LV_LABEL_LONG_WRAP);
  lv_obj_align(men_info, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(men_info, &lv_font_montserrat_20, 0);
  lv_label_set_text_fmt(men_info, "80%%");
  // lv_obj_set_style_text_color(hume, lv_color_hex(0x000000), 0);
  lv_obj_set_style_opa(men_info, opa_default, 0);

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, arc_men);
  lv_anim_set_exec_cb(&a, set_angle);
  lv_anim_set_time(&a, 500);
  lv_anim_set_repeat_count(&a, 1);
  lv_anim_set_repeat_delay(&a, 500);
  lv_anim_set_values(&a, 0, 50);
  lv_anim_start(&a);

  lv_obj_t *mem_info;
  mem_info = lv_label_create(obj4);
  lv_label_set_long_mode(mem_info, LV_LABEL_LONG_WRAP);
  lv_obj_align(mem_info, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_text_font(mem_info, &lv_font_montserrat_20, 0);
  lv_label_set_text_fmt(mem_info, "%dK / %dK", 200, 400);
  // lv_obj_set_style_text_color(hume, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(mem_info, 0, 0);
  lv_obj_set_style_text_opa(mem_info, 255, 0);

  /*     static const char * btns[] ={""};

    lv_obj_t *mbox1 = lv_msgbox_create(img, "求三连", "画板子不易\n给个点赞吧求求了\n关掉后不会再次弹出", btns, true);
    lv_obj_add_event_cb(mbox1, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_style_text_font(mbox1, &xiaolai_20, 0);
    lv_obj_center(mbox1);

    lv_color_t bg_color_1 = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
    lv_color_t fg_color_1 = lv_palette_darken(LV_PALETTE_BLUE, 4);

    lv_obj_t *qr1 = lv_qrcode_create(mbox1, 200, fg_color_1, bg_color_1);

    const char *data1 = "https://oshwhub.com/an_ye/zhuo-mian-fu-ping";
    lv_qrcode_update(qr1, data1, strlen(data1));
    lv_obj_align(qr1, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_set_style_border_color(qr1, bg_color_1, 0);
    lv_obj_set_style_border_width(qr1, 5, 0);
    lv_obj_set_size(qr1,200,200); */

  large_obj1 = lv_obj_create(screen1);
  lv_obj_set_size(large_obj1, 460, 120);
  lv_obj_align(large_obj1, LV_ALIGN_TOP_MID, 0, 650);
  lv_obj_set_style_opa(large_obj1, opa_default, 0);

  screen_br = lv_slider_create(obj3);
  lv_obj_set_size(screen_br, 40, 160);
  lv_slider_set_range(screen_br, 100, 1023);
  lv_obj_align(screen_br, LV_ALIGN_BOTTOM_RIGHT, -10, 0);
  lv_obj_add_event_cb(screen_br, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  static lv_style_t evn_br_style;

  lv_style_init(&evn_br_style);
  lv_style_set_bg_opa(&evn_br_style, LV_OPA_COVER);
  lv_style_set_bg_color(&evn_br_style, lv_palette_main(LV_PALETTE_RED));
  lv_style_set_bg_grad_color(&evn_br_style, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_bg_grad_dir(&evn_br_style, LV_GRAD_DIR_VER);

  evn_br = lv_bar_create(obj3);
  lv_obj_add_style(evn_br, &evn_br_style, LV_PART_INDICATOR);
  lv_obj_set_size(evn_br, 40, 160);
  lv_obj_align(evn_br, LV_ALIGN_BOTTOM_LEFT, 10, 0);
  lv_bar_set_range(evn_br, 0, 400);

  lv_anim_init(&evn_br_anim);
  lv_anim_set_exec_cb(&evn_br_anim, set_br);
  lv_anim_set_time(&evn_br_anim, 1000);
  lv_anim_set_repeat_count(&evn_br_anim, 1);
  lv_anim_set_repeat_delay(&evn_br_anim, 500);
  lv_anim_set_var(&evn_br_anim, evn_br);
  lv_anim_set_values(&evn_br_anim, 0, 300);
  lv_anim_start(&evn_br_anim);

  label_evn_br = lv_label_create(obj3);
  lv_label_set_long_mode(label_evn_br, LV_LABEL_LONG_WRAP);
  lv_obj_align(label_evn_br, LV_ALIGN_CENTER, 0, -40);
  lv_obj_set_style_text_font(label_evn_br, &xiaolai_20, 0);
  lv_label_set_text_fmt(label_evn_br, " 光强度:\n%.1flux", 100.1);
  lv_obj_set_style_bg_opa(label_evn_br, 0, 0);
  lv_obj_set_style_text_opa(label_evn_br, 255, 0);

  label_screen_br = lv_label_create(obj3);
  lv_label_set_long_mode(label_screen_br, LV_LABEL_LONG_WRAP);
  lv_obj_align(label_screen_br, LV_ALIGN_CENTER, 0, 40);
  lv_obj_set_style_text_font(label_screen_br, &xiaolai_20, 0);
  lv_label_set_text_fmt(label_screen_br, "背光亮度:\n  %d%%", 99);
  lv_obj_set_style_bg_opa(label_screen_br, 0, 0);
  lv_obj_set_style_text_opa(label_screen_br, 255, 0);

  /*   uint32_t i;
  for (i = 0; i < 20; i++)
  {
    lv_obj_t *btn = lv_btn_create(obj3);
    lv_obj_set_width(btn, lv_pct(100));

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text_fmt(label, LV_SYMBOL_BLUETOOTH " test %d", i);
  }

  lv_event_send(obj3, LV_EVENT_SCROLL, NULL);

  lv_obj_scroll_to_view(lv_obj_get_child(obj3, 0), LV_ANIM_OFF);
  */

  /*
    lv_timer_t *timer_temp = lv_timer_create(get_temperature_value, 2000, NULL);
    lv_timer_set_repeat_count(timer_temp, -1);
    lv_timer_ready(timer_temp);

    lv_timer_t *timer_hume = lv_timer_create(get_humidity_value, 2000, NULL);
    lv_timer_set_repeat_count(timer_hume, -1);
    lv_timer_ready(timer_hume);

    lv_timer_t *timer_batter = lv_timer_create(get_bat_value, 1000, NULL);
    lv_timer_set_repeat_count(timer_batter, -1);
    lv_timer_ready(timer_batter);

    lv_timer_t *timer_br = lv_timer_create(get_br_value, 1000, NULL);
    lv_timer_set_repeat_count(timer_br, -1);
    lv_timer_ready(timer_br);

    lv_timer_t *timer_esp = lv_timer_create(get_esp_info, 10000, NULL);
    lv_timer_set_repeat_count(timer_esp, -1);
*/

  lv_timer_t *timer_weather = lv_timer_create(get_weather_forecast, 10 * 3600 * 1000, NULL);
  lv_timer_set_repeat_count(timer_weather, -1);
  lv_timer_ready(timer_weather);

  lv_timer_t *timer_date = lv_timer_create(get_time_value, 1000, NULL);
  lv_timer_set_repeat_count(timer_date, -1);
  lv_timer_ready(timer_date);
}

#define MAX_HTTP_OUTPUT_BUFFER 1300
#define HOST "api.seniverse.com"
#define UserKey "SNMe_2JQN4QUEwh6U"
#define Location "ip"
#define Language "zh"
#define Strat "0"
#define Days "5"

static void http_client_task(void *parm)
{
  vTaskDelay(pdMS_TO_TICKS(1000));
  char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0}; // Buffer to store response of http request
  int content_length = 0;
  static const char *URL = "http://api.seniverse.com/v3/weather/daily.json?key=SNMe_2JQN4QUEwh6U&location=ip&language=zh-Hans&unit=c&days=3";
  lv_textarea_add_text(log_textarea, "htttp通讯开始");
  lv_textarea_add_char(log_textarea, '\n');
  esp_http_client_config_t config = {
      .url = URL,
  };
  esp_http_client_handle_t client = esp_http_client_init(&config);

  // GET Request
  esp_http_client_set_method(client, HTTP_METHOD_GET);
  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK)
  {
    lv_textarea_add_text(log_textarea, "http失败原因");
    lv_textarea_add_char(log_textarea, '\n');
    lv_textarea_add_text(log_textarea, esp_err_to_name(err));
    lv_textarea_add_char(log_textarea, '\n');

    // ESP_LOGE(HTTP_TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
  }
  else
  {
    content_length = esp_http_client_fetch_headers(client);
    if (content_length < 0)
    {
      // ESP_LOGE(HTTP_TAG, "HTTP client fetch headers failed");
    }
    else
    {
      int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
      if (data_read >= 0)
      {
        lv_textarea_add_text(log_textarea, "htttp成功");
        lv_textarea_add_char(log_textarea, '\n');
        // printf("data:%s", output_buffer);

        lv_textarea_add_text(log_textarea, output_buffer);
        lv_textarea_add_char(log_textarea, '\n');
        char *date;
        root = cJSON_Parse(output_buffer);

        cJSON *cjson_item = cJSON_GetObjectItem(root, "results");
        cJSON *cjson_results = cJSON_GetArrayItem(cjson_item, 0);
        cJSON *cjson_now = cJSON_GetObjectItem(cjson_results, "location");
        cJSON *cjson_temperature = cJSON_GetObjectItem(cjson_now, "name");
        date = cjson_temperature->valuestring;
        lv_textarea_add_text(log_textarea, "ip所在地区:");
        lv_textarea_add_char(log_textarea, '\n');
        lv_textarea_add_text(log_textarea, date);
        lv_textarea_add_char(log_textarea, '\n');
      }
      else
      {
        // ESP_LOGE(HTTP_TAG, "Failed to read response");
      }
    }
  }
  esp_http_client_close(client);
  vTaskDelete(NULL);
}

static void smartconfig_example_task(void *parm);

static void event_start(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED)
  {
    lv_textarea_add_text(log_textarea, "完成设置");
    lv_textarea_add_char(log_textarea, '\n'); // 换行
    lv_obj_add_flag(tv, LV_OBJ_FLAG_HIDDEN);
    main_ui();
    anim_main_ui();
    lv_anim_timeline_start(anim_timeline);
  }
  else if (code == LV_EVENT_VALUE_CHANGED)
  {
    LV_LOG_USER("Toggled");
  }
}

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
bool enable_wifi_smartconfig = false;

void fun()
{
  time_t now = 0;
  struct tm timeinfo = {0};

  time(&now);
  localtime_r(&now, &timeinfo);

  char str[64];
  strftime(str, sizeof(str), "%c", &timeinfo);
  ESP_LOGI("TAG", "time updated: %s", str);

  lv_textarea_add_text(log_textarea, str);
  lv_textarea_add_char(log_textarea, '\n');
}
int fail_connect_count = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
  wifi_config_t wifi_config;

  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
      // NVS partition was truncated and needs to be erased
      // Retry nvs_flash_init
      ESP_ERROR_CHECK(nvs_flash_erase());
      err = nvs_flash_init();
      lv_textarea_add_text(log_textarea, "读取失败,清除所有数据");
      lv_textarea_add_char(log_textarea, '\n');
    }
    ESP_ERROR_CHECK(err);
    esp_wifi_get_config(WIFI_IF_STA, &wifi_config);

    if (enable_wifi_smartconfig)
    {
      xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
      enable_wifi_smartconfig = false;
    }
    else
    {
      lv_textarea_add_text(log_textarea, "保存配置读取成功");
      lv_textarea_add_char(log_textarea, '\n');
      esp_wifi_connect();
      xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
      xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
  }
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    lv_textarea_add_text(log_textarea, "连接失败,正在重试");
    lv_textarea_add_char(log_textarea, '\n');
    fail_connect_count++;
    if (fail_connect_count < 5)
    {
      esp_wifi_connect();
    }
    else
    {
      xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
    }

    xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    lv_textarea_add_text(log_textarea, "获取ip成功");
    lv_textarea_add_char(log_textarea, '\n');
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "ntp.aliyun.com"); // 设置访问服务器
    sntp_setservername(1, "pool.ntp.org");
    sntp_setservername(2, "210.72.145.44");
    setenv("TZ", "CST-8", 1);
    tzset();
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    vTaskDelay(pdMS_TO_TICKS(1000));
    sntp_init();

    lv_textarea_add_text(log_textarea, "ntp同步成功");
    lv_textarea_add_char(log_textarea, '\n');
    fun();
    xTaskCreate(http_client_task, "http_client", 5120, NULL, 3, NULL);
    //  get_w_update();

    xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
  }
  else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
  {
    lv_textarea_add_text(log_textarea, "Scan done");
    lv_textarea_add_char(log_textarea, '\n');
  }
  else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
  {
    lv_textarea_add_text(log_textarea, "Found channel");
    lv_textarea_add_char(log_textarea, '\n');
  }
  else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
  {
    lv_textarea_add_text(log_textarea, "获取配网信息");
    lv_textarea_add_char(log_textarea, '\n');

    smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;

    uint8_t ssid[33] = {0};
    uint8_t password[65] = {0};
    uint8_t rvd_data[33] = {0};

    bzero(&wifi_config, sizeof(wifi_config_t));
    memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
    wifi_config.sta.bssid_set = evt->bssid_set;
    if (wifi_config.sta.bssid_set == true)
    {
      memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
    }

    memcpy(ssid, evt->ssid, sizeof(evt->ssid));
    memcpy(password, evt->password, sizeof(evt->password));
    ESP_LOGI(TAG, "SSID:%s", ssid);
    ESP_LOGI(TAG, "PASSWORD:%s", password);
    if (evt->type == SC_TYPE_ESPTOUCH_V2)
    {
      ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
      ESP_LOGI(TAG, "RVD_DATA:");
      for (int i = 0; i < 33; i++)
      {
        printf("%02x ", rvd_data[i]);
      }
      printf("\n");
    }

    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_wifi_connect();
  }
  else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
  {
    xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
  }
}

static void initialise_wifi(void)
{
  lv_textarea_add_text(log_textarea, "进入wifi配置");
  lv_textarea_add_char(log_textarea, '\n');

  ESP_ERROR_CHECK(esp_netif_init());
  s_wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
}

static void smartconfig_example_task(void *parm)
{
  EventBits_t uxBits;
  ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
  smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
  while (1)
  {
    uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
    if (uxBits & CONNECTED_BIT)
    {
      lv_textarea_add_text(log_textarea, "连接到ap");
      lv_textarea_add_char(log_textarea, '\n');
    }
    if (uxBits & ESPTOUCH_DONE_BIT)
    {
      lv_textarea_add_text(log_textarea, "配网结束");
      lv_textarea_add_char(log_textarea, '\n');
      esp_smartconfig_stop();
      vTaskDelete(NULL);
    }
  }
}

static void event_connect_wifi(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED)
  {
    lv_textarea_add_text(log_textarea, "正在连接wifi");
    lv_textarea_add_char(log_textarea, '\n');

    char buf[32];
    lv_dropdown_get_selected_str(dd, buf, sizeof(buf));
    wifi_ssid = buf;
    lv_textarea_add_text(log_textarea, buf);
    lv_textarea_add_char(log_textarea, '\n');

    const char *test = (const char *)lv_textarea_get_text(pwd_ta);
    wifi_password = &test;
    lv_textarea_add_text(log_textarea, test);
    lv_textarea_add_char(log_textarea, '\n');

    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi();
  }
  else if (code == LV_EVENT_VALUE_CHANGED)
  {
    LV_LOG_USER("Toggled");
  }
}

static void event_reset_time(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED)
  {
    lv_textarea_add_text(log_textarea, "同步ntp时间");
    lv_textarea_add_char(log_textarea, '\n');
    fun();
  }
  else if (code == LV_EVENT_VALUE_CHANGED)
  {
    LV_LOG_USER("Toggled");
  }
}

static void event_reset_net(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED)
  {
    lv_textarea_add_text(log_textarea, "清除所有数据");
    lv_textarea_add_char(log_textarea, '\n');
    ESP_ERROR_CHECK(nvs_flash_erase());
    lv_textarea_add_text(log_textarea, "进入配网模式");
    lv_textarea_add_char(log_textarea, '\n');
    enable_wifi_smartconfig = true;
    // xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
  }
  else if (code == LV_EVENT_VALUE_CHANGED)
  {
    LV_LOG_USER("Toggled");
  }
}

void ui_setup()
{
  LV_FONT_DECLARE(xiaolai_48);
  LV_FONT_DECLARE(xiaolai_20);

  static lv_style_t style;
  lv_style_init(&style);

  lv_style_set_radius(&style, 3);

  lv_style_set_bg_opa(&style, LV_OPA_100);
  lv_style_set_bg_color(&style, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_bg_grad_color(&style, lv_palette_darken(LV_PALETTE_BLUE, 2));
  lv_style_set_bg_grad_dir(&style, LV_GRAD_DIR_VER);

  lv_style_set_border_opa(&style, LV_OPA_40);
  lv_style_set_border_width(&style, 2);
  lv_style_set_border_color(&style, lv_palette_main(LV_PALETTE_GREY));

  lv_style_set_shadow_width(&style, 8);
  lv_style_set_shadow_color(&style, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_shadow_ofs_y(&style, 8);

  lv_style_set_outline_opa(&style, LV_OPA_COVER);
  lv_style_set_outline_color(&style, lv_palette_main(LV_PALETTE_BLUE));

  lv_style_set_text_color(&style, lv_color_white());
  lv_style_set_pad_all(&style, 10);

  /*Init the pressed style*/
  static lv_style_t style_pr;
  lv_style_init(&style_pr);

  /*Add a large outline when pressed*/
  lv_style_set_outline_width(&style_pr, 30);
  lv_style_set_outline_opa(&style_pr, LV_OPA_TRANSP);

  lv_style_set_translate_y(&style_pr, 5);
  lv_style_set_shadow_ofs_y(&style_pr, 3);
  lv_style_set_bg_color(&style_pr, lv_palette_darken(LV_PALETTE_BLUE, 2));
  lv_style_set_bg_grad_color(&style_pr, lv_palette_darken(LV_PALETTE_BLUE, 4));

  LV_IMG_DECLARE(background);
  img = lv_img_create(lv_scr_act());
  lv_img_set_src(img, &background);

  log_textarea = lv_textarea_create(img);
  lv_obj_set_style_text_font(log_textarea, &xiaolai_20, 0);
  lv_obj_set_size(log_textarea, 150, 300); // 设置文本框尺寸为 100x200
  lv_obj_set_style_opa(log_textarea, opa_default, 0);
  lv_obj_align(log_textarea, LV_ALIGN_LEFT_MID, 0, 0); // 在左侧中间对齐
  lv_textarea_set_text(log_textarea, "");              // 初始文本为空

  lv_textarea_add_text(log_textarea, "初始化正常");
  lv_textarea_add_char(log_textarea, '\n'); // 换行

  lv_textarea_add_text(log_textarea, "GT911...电容触摸已识别");
  lv_textarea_add_char(log_textarea, '\n'); // 换行

  lv_textarea_add_text(log_textarea, "SHT31...温湿度传感器已识别");
  lv_textarea_add_char(log_textarea, '\n'); // 换行

  screen1 = lv_obj_create(img);
  lv_obj_set_size(screen1, 800, 480);
  lv_obj_align(screen1, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_opa(screen1, 0, 0);
  lv_obj_set_style_border_opa(screen1, 0, 0);
  lv_obj_set_scrollbar_mode(screen1, LV_SCROLLBAR_MODE_OFF);
  lv_obj_add_flag(screen1, LV_OBJ_FLAG_SCROLL_MOMENTUM);
  // lv_obj_set_style_opa(screen1, 0, 0);

  tv = lv_tileview_create(screen1);
  lv_obj_set_size(tv, 400, 450);
  lv_obj_center(tv);
  lv_obj_set_style_opa(tv, opa_default, 0);
  /*Tile1: just a label*/
  lv_obj_t *tile1 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_BOTTOM);

  lv_obj_t *label1 = lv_label_create(tv);
  lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);
  lv_label_set_recolor(label1, true);
  lv_obj_set_style_text_font(label1, &xiaolai_20, 0); /*Enable re-coloring by commands in the text*/
  lv_label_set_text(label1,
                    "#ff0000 设置#\n\n"
                    "#00ff00 可以在这里修改和保存配置信息#\n"
                    "\n继续请向下滑动\n");
  lv_obj_set_width(label1, 300);
  lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label1);

  /*Tile2: a button*/
  lv_obj_t *tile2 = lv_tileview_add_tile(tv, 0, 1, LV_DIR_TOP | LV_DIR_RIGHT);

  lv_color_t bg_color_1 = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
  lv_color_t fg_color_1 = lv_palette_darken(LV_PALETTE_BLUE, 4);

  lv_obj_t *qr1 = lv_qrcode_create(tile2, 200, fg_color_1, bg_color_1);

  const char *data1 = "https://oshwhub.com/an_ye/7cun-zhuo-mian-xin-xi-ping";
  lv_qrcode_update(qr1, data1, strlen(data1));
  lv_obj_align(qr1, LV_ALIGN_CENTER, 0, -30);

  lv_obj_set_style_border_color(qr1, bg_color_1, 0);
  lv_obj_set_style_border_width(qr1, 5, 0);
  lv_obj_set_size(qr1, 200, 200);

  lv_obj_t *label2 = lv_label_create(tile2);
  lv_label_set_long_mode(label2, LV_LABEL_LONG_WRAP);
  lv_label_set_recolor(label2, true);
  lv_obj_set_style_text_font(label2, &xiaolai_20, 0); /*Enable re-coloring by commands in the text*/
  lv_label_set_text(label2, "肝项目不易,给个赞吧求求了\n\n向右侧滑动开始配置");
  lv_obj_set_width(label2, 300);
  lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label2, LV_ALIGN_BOTTOM_MID, 0, -50);

  lv_obj_t *tile3 = lv_tileview_add_tile(tv, 1, 1, LV_DIR_LEFT | LV_DIR_RIGHT);

  lv_obj_t *label3 = lv_label_create(tile3);
  lv_label_set_long_mode(label3, LV_LABEL_LONG_WRAP);
  lv_label_set_recolor(label3, true);
  lv_obj_set_style_text_font(label3, &xiaolai_20, 0); /*Enable re-coloring by commands in the text*/
  lv_label_set_text(label3, "设置界面透明度\n默认为180\n可设定范围50-255\n");
  lv_obj_set_width(label3, 300);
  lv_obj_set_style_text_align(label3, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label3, LV_ALIGN_TOP_MID, 0, 20);

  screen_opa = lv_slider_create(tile3);
  lv_obj_set_size(screen_opa, 40, 220);
  lv_slider_set_range(screen_opa, 50, 255);
  lv_slider_set_value(screen_opa, 180, LV_ANIM_ON);
  lv_obj_align(screen_opa, LV_ALIGN_CENTER, 0, 30);
  lv_obj_add_event_cb(screen_opa, opa_cb, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *tile4 = lv_tileview_add_tile(tv, 2, 1, LV_DIR_LEFT | LV_DIR_RIGHT);

  lv_obj_t *label4 = lv_label_create(tile4);
  lv_label_set_long_mode(label4, LV_LABEL_LONG_WRAP);
  lv_label_set_recolor(label4, true);
  lv_obj_set_style_text_font(label4, &xiaolai_20, 0);
  lv_label_set_text(label4, "请在下表选择wifi并输入密码\n\n注意,仅能连接2.4Ghz wifi");
  lv_obj_set_width(label4, 300);
  lv_obj_set_style_text_align(label4, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label4, LV_ALIGN_TOP_MID, 0, 20);

  dd = lv_dropdown_create(tile4);
  lv_dropdown_set_options(dd, "请选择WIFI");
  lv_obj_set_style_text_font(dd, &xiaolai_20, 0);

  lv_obj_align(dd, LV_ALIGN_CENTER, 0, -30);
  lv_obj_set_size(dd, 300, 40);
  lv_obj_add_event_cb(dd, event_dd, LV_EVENT_ALL, NULL);

  pwd_ta = lv_textarea_create(tile4);
  lv_textarea_set_text(pwd_ta, "");
  lv_textarea_set_password_mode(pwd_ta, true);
  lv_textarea_set_one_line(pwd_ta, true);
  lv_obj_set_size(pwd_ta, 300, 40);
  lv_obj_align(pwd_ta, LV_ALIGN_CENTER, 0, 60);
  lv_obj_add_event_cb(pwd_ta, ta_event_cb, LV_EVENT_ALL, NULL);

  lv_obj_t *pwd_label = lv_label_create(tile4);
  lv_obj_set_style_text_font(pwd_label, &xiaolai_20, 0);
  lv_label_set_text(pwd_label, "Wifi 密码:");
  lv_obj_align_to(pwd_label, pwd_ta, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

  lv_obj_t *btn8 = lv_btn_create(tile4);
  lv_obj_remove_style_all(btn8);
  lv_obj_add_style(btn8, &style, 0);
  lv_obj_add_style(btn8, &style_pr, LV_STATE_PRESSED);
  lv_obj_set_size(btn8, 150, 40);
  lv_obj_add_event_cb(btn8, event_connect_wifi, LV_EVENT_ALL, NULL);
  lv_obj_align(btn8, LV_ALIGN_CENTER, 100, 150);

  lv_obj_t *label8 = lv_label_create(btn8);
  lv_label_set_text(label8, "连接");
  lv_obj_set_style_text_font(label8, &xiaolai_20, 0);
  lv_obj_center(label8);

  lv_obj_t *btn9 = lv_btn_create(tile4);
  lv_obj_remove_style_all(btn9);
  lv_obj_add_style(btn9, &style, 0);
  lv_obj_add_style(btn9, &style_pr, LV_STATE_PRESSED);
  lv_obj_set_size(btn9, 150, 40);
  lv_obj_add_event_cb(btn9, event_reset_net, LV_EVENT_ALL, NULL);
  lv_obj_align(btn9, LV_ALIGN_CENTER, -100, 150);

  lv_obj_t *label9 = lv_label_create(btn9);
  lv_label_set_text(label9, "重置");
  lv_obj_set_style_text_font(label9, &xiaolai_20, 0);
  lv_obj_center(label9);

  kb = lv_keyboard_create(lv_scr_act());
  lv_obj_set_size(kb, LV_HOR_RES, 160);
  lv_keyboard_set_textarea(kb, pwd_ta);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *tile5 = lv_tileview_add_tile(tv, 3, 1, LV_DIR_LEFT | LV_DIR_RIGHT);

  lv_color_t bg_color_2 = lv_palette_lighten(LV_PALETTE_ORANGE, 5);
  lv_color_t fg_color_2 = lv_palette_darken(LV_PALETTE_LIGHT_GREEN, 4);
  lv_obj_t *qr2 = lv_qrcode_create(tile5, 200, fg_color_2, bg_color_2);
  const char *data2 = "http://wx.ai-thinker.com/api/old/wifi/config";
  lv_qrcode_update(qr2, data2, strlen(data2));
  lv_obj_align(qr2, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_border_color(qr2, bg_color_2, 0);
  lv_obj_set_style_border_width(qr2, 5, 0);

  lv_obj_t *label5 = lv_label_create(tile5);
  lv_label_set_long_mode(label5, LV_LABEL_LONG_WRAP);
  lv_label_set_recolor(label5, true);
  lv_obj_set_style_text_font(label5, &xiaolai_20, 0);
  lv_label_set_text(label5, "微信扫码自动配网\n(备用配网方案)\n如果已经连接wifi请跳过");
  lv_obj_set_width(label5, 300);
  lv_obj_set_style_text_align(label5, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label5, LV_ALIGN_TOP_MID, 0, 20);

  lv_obj_t *tile6 = lv_tileview_add_tile(tv, 4, 1, LV_DIR_LEFT | LV_DIR_RIGHT);

  lv_obj_t *label6 = lv_label_create(tile6);
  lv_label_set_long_mode(label6, LV_LABEL_LONG_WRAP);
  lv_label_set_recolor(label6, true);
  lv_obj_set_style_text_font(label6, &xiaolai_20, 0);
  lv_label_set_text(label6, "和风天气配置\n\n请前往和风天气控制台,获取以下信息");
  lv_obj_set_width(label6, 300);
  lv_obj_set_style_text_align(label6, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label6, LV_ALIGN_TOP_MID, 0, 20);

  lv_obj_t *text6_UserKey = lv_textarea_create(tile6);
  lv_textarea_set_one_line(text6_UserKey, true);
  lv_textarea_set_password_mode(text6_UserKey, false);
  lv_obj_set_size(text6_UserKey, 200, 40);
  lv_obj_add_event_cb(text6_UserKey, ta_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_align(text6_UserKey, LV_ALIGN_TOP_MID, 0, 130);

  lv_obj_t *label6_UserKey = lv_label_create(tile6);
  lv_label_set_text(label6_UserKey, "UserKey:");
  lv_obj_align_to(label6_UserKey, text6_UserKey, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

  lv_obj_t *text6_Location = lv_textarea_create(tile6);
  lv_textarea_set_one_line(text6_Location, true);
  lv_textarea_set_password_mode(text6_Location, false);
  lv_obj_set_size(text6_Location, 200, 40);
  lv_obj_add_event_cb(text6_Location, ta_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_align(text6_Location, LV_ALIGN_TOP_MID, 0, 200);

  lv_obj_t *label6_Location = lv_label_create(tile6);
  lv_obj_set_style_text_font(label6_Location, &xiaolai_20, 0);
  lv_label_set_text(label6_Location, "地区,默认自动获取");
  lv_obj_align_to(label6_Location, text6_Location, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

  lv_obj_t *dd_Unit = lv_dropdown_create(tile6);
  lv_dropdown_set_options(dd_Unit, "m\ni");
  lv_obj_set_size(dd_Unit, 200, 40);
  lv_obj_align(dd_Unit, LV_ALIGN_TOP_MID, 0, 270);
  lv_obj_add_event_cb(dd_Unit, event_dd, LV_EVENT_ALL, NULL);

  lv_obj_t *label6_Unit = lv_label_create(tile6);
  lv_label_set_text(label6_Unit, "公制-m/英制-i");
  lv_obj_set_style_text_font(label6_Unit, &xiaolai_20, 0);
  lv_obj_align_to(label6_Unit, dd_Unit, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

  lv_obj_t *dd_Lang = lv_dropdown_create(tile6);
  lv_dropdown_set_options(dd_Lang, "zh\nen");
  lv_obj_set_size(dd_Lang, 200, 40);
  lv_obj_align(dd_Lang, LV_ALIGN_TOP_MID, 0, 340);
  lv_obj_add_event_cb(dd_Lang, event_dd, LV_EVENT_ALL, NULL);

  lv_obj_t *label6_Lang = lv_label_create(tile6);
  lv_label_set_text(label6_Lang, "语言 英文-en/中文-zh");
  lv_obj_set_style_text_font(label6_Lang, &xiaolai_20, 0);
  lv_obj_align_to(label6_Lang, dd_Lang, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

  lv_obj_t *tile7 = lv_tileview_add_tile(tv, 5, 1, LV_DIR_LEFT | LV_DIR_RIGHT);

  lv_obj_t *label7_t = lv_label_create(tile7);
  lv_label_set_long_mode(label7_t, LV_LABEL_LONG_WRAP);
  lv_label_set_recolor(label7_t, true);
  lv_obj_set_style_text_font(label7_t, &xiaolai_20, 0);
  lv_label_set_text(label7_t, "时间配置\n\n请核对以下信息");
  lv_obj_set_width(label7_t, 300);
  lv_obj_set_style_text_align(label7_t, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label7_t, LV_ALIGN_TOP_MID, 0, 20);

  lv_obj_t *dd_server = lv_dropdown_create(tile7);
  lv_dropdown_set_options(dd_server, "pool.ntp.org\nntp.aliyun.com");
  lv_obj_set_size(dd_server, 200, 40);
  lv_obj_align(dd_server, LV_ALIGN_TOP_MID, 0, 150);
  lv_obj_add_event_cb(dd_server, event_dd, LV_EVENT_ALL, NULL);

  lv_obj_t *label7_server = lv_label_create(tile7);
  lv_label_set_text(label7_server, "ntp服务器地址");
  lv_obj_set_style_text_font(label7_server, &xiaolai_20, 0);
  lv_obj_align_to(label7_server, dd_server, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

  lv_obj_t *dd_offset = lv_dropdown_create(tile7);
  lv_dropdown_set_options(dd_offset, "UTC+8\nUTC+0");
  lv_obj_set_size(dd_offset, 200, 40);
  lv_obj_align(dd_offset, LV_ALIGN_TOP_MID, 0, 230);
  lv_obj_add_event_cb(dd_offset, event_dd, LV_EVENT_ALL, NULL);

  lv_obj_t *label7_offset = lv_label_create(tile7);
  lv_label_set_text(label7_offset, "时区");
  lv_obj_set_style_text_font(label7_offset, &xiaolai_20, 0);
  lv_obj_align_to(label7_offset, dd_offset, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

  lv_obj_t *btn10 = lv_btn_create(tile7);
  lv_obj_remove_style_all(btn10);
  lv_obj_add_style(btn10, &style, 0);
  lv_obj_add_style(btn10, &style_pr, LV_STATE_PRESSED);
  lv_obj_set_size(btn10, 200, 40);
  lv_obj_add_event_cb(btn10, event_reset_time, LV_EVENT_ALL, NULL);
  lv_obj_align(btn10, LV_ALIGN_CENTER, 0, 150);

  lv_obj_t *label10 = lv_label_create(btn10);
  lv_obj_set_style_text_font(label10, &xiaolai_20, 0);
  lv_label_set_text(label10, "更新时间");
  lv_obj_center(label10);

  lv_obj_t *tile8 = lv_tileview_add_tile(tv, 6, 1, LV_DIR_LEFT | LV_DIR_RIGHT);

  /*Add a transition to the outline*/
  static lv_style_transition_dsc_t trans;
  static lv_style_prop_t props[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA, (lv_style_prop_t)0};
  lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0, NULL);

  lv_style_set_transition(&style_pr, &trans);

  lv_obj_t *btn7 = lv_btn_create(tile8);
  lv_obj_remove_style_all(btn7);
  lv_obj_add_style(btn7, &style, 0);
  lv_obj_add_style(btn7, &style_pr, LV_STATE_PRESSED);
  lv_obj_set_size(btn7, 150, 40);
  lv_obj_add_event_cb(btn7, event_start, LV_EVENT_ALL, NULL);
  lv_obj_center(btn7);

  lv_obj_t *label7 = lv_label_create(btn7);
  lv_label_set_text(label7, "开始");
  lv_obj_set_style_text_font(label7, &xiaolai_20, 0);
  lv_obj_center(label7);

  lv_timer_handler();

  // wifi_scan();
  /*
    lv_obj_t *obj1_1;
    obj1_1 = lv_obj_create(screen1);
    lv_obj_set_size(obj1_1, 320, 320);
    lv_obj_align(obj1_1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_opa(obj1_1, opa_default + 30, 0);

    lv_obj_t *spinner2_1 = lv_spinner_create(obj1_1, 1000, 60);
    lv_obj_set_size(spinner2_1, 150, 150);
    lv_obj_align(spinner2_1, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *wifi_state;
    wifi_state = lv_label_create(obj1_1);
    lv_label_set_long_mode(wifi_state, LV_LABEL_LONG_WRAP);
    lv_obj_align(wifi_state, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_text_font(wifi_state, &xiaolai_48, 0);
    lv_label_set_text(wifi_state, "连接WIFI中");
    lv_obj_set_style_text_color(wifi_state, lv_color_hex(0x66CCFF), 0);
    lv_obj_set_style_bg_opa(wifi_state, 0, 0);
    lv_obj_set_style_text_opa(wifi_state, 255, 0);

    lv_timer_handler();

    for (int i = 0; i < 500; i++)
    {
      lv_timer_handler();
      usleep(5 * 1000);
    }

    lv_obj_add_flag(spinner2_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_color(wifi_state, lv_color_hex(0x00FF00), 0);
    lv_label_set_text(wifi_state, "连接成功");
    lv_obj_align(wifi_state, LV_ALIGN_CENTER, 0, 0);

    for (int i = 0; i < 500; i++)
    {
      lv_timer_handler();
      usleep(5 * 1000);
    }

    lv_label_set_text(wifi_state, "连接失败\n进入自动配网");
    lv_obj_set_style_text_align(wifi_state, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(wifi_state, lv_color_hex(0xFF0000), 0);
    lv_obj_align(wifi_state, LV_ALIGN_CENTER, 0, 0);

    for (int i = 0; i < 100; i++)
    {
      lv_timer_handler();
      usleep(5 * 1000);
    }

    lv_label_set_text(wifi_state, "请打开手机微信\n扫一扫上方二维码\n或下载EspTorch\n开始自动配网");
    lv_obj_set_style_text_align(wifi_state, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(wifi_state, lv_color_hex(0xFF0000), 0);
    lv_obj_set_size(obj1_1, 460, 740);
    lv_obj_align(wifi_state, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *tmp_text;
    tmp_text = lv_label_create(obj1_1);
    lv_label_set_long_mode(tmp_text, LV_LABEL_LONG_WRAP);
    lv_obj_align(tmp_text, LV_ALIGN_CENTER, 20, 100);
    lv_obj_set_style_text_font(tmp_text, &xiaolai_20, 0);
    lv_label_set_text(tmp_text, "等待获取smartconfig信息中...");
    lv_obj_set_style_text_color(tmp_text, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(tmp_text, 0, 0);

    lv_obj_t *spinner2_2 = lv_spinner_create(obj1_1, 1000, 60);
    lv_obj_set_size(spinner2_2, 50, 50);
    lv_obj_align(spinner2_2, LV_ALIGN_CENTER, -150, 100);

    lv_color_t bg_color_1 = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
    lv_color_t fg_color_1 = lv_palette_darken(LV_PALETTE_BLUE, 4);

    lv_obj_t *qr1 = lv_qrcode_create(obj1_1, 200, fg_color_1, bg_color_1);

    const char *data1 = "https://www.espressif.com.cn/zh-hans/products/software/esp-touch/resources";
    lv_qrcode_update(qr1, data1, strlen(data1));
    lv_obj_align(qr1, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_set_style_border_color(qr1, bg_color_1, 0);
    lv_obj_set_style_border_width(qr1, 5, 0);

    lv_color_t bg_color_2 = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
    lv_color_t fg_color_2 = lv_palette_darken(LV_PALETTE_BLUE, 4);

    lv_obj_t *qr2 = lv_qrcode_create(obj1_1, 200, fg_color_2, bg_color_2);

    const char *data2 = "http://wx.ai-thinker.com/api/old/wifi/config";
    lv_qrcode_update(qr2, data2, strlen(data2));
    lv_obj_align(qr2, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_set_style_border_color(qr2, bg_color_2, 0);
    lv_obj_set_style_border_width(qr2, 5, 0);
   */
}

void app_main(void)
{

  esp_lcd_panel_io_handle_t tp_io_handle = NULL;

  const esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
  const esp_lcd_touch_config_t tp_cfg = {
      .x_max = EXAMPLE_LCD_H_RES,
      .y_max = EXAMPLE_LCD_V_RES,
      .rst_gpio_num = GPIO_NUM_NC,
      .int_gpio_num = GPIO_NUM_NC,
      .levels = {
          .reset = 0,
          .interrupt = 0,
      },
      .flags = {
          .swap_xy = 0,
          .mirror_x = 0,
          .mirror_y = 0,
      },
  };

  int i2c_master_port = I2C_MASTER_NUM;

  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));

  ESP_ERROR_CHECK(i2c_param_config(1, &conf));
  ESP_ERROR_CHECK(i2c_driver_install(1, conf.mode, 0, 0, 0));

  ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)1, &tp_io_config, &tp_io_handle));
  ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &tp_handle));

  static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
  static lv_disp_drv_t disp_drv;      // contains callback functions

#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
  ESP_LOGI(TAG, "Create semaphores");
  sem_vsync_end = xSemaphoreCreateBinary();
  assert(sem_vsync_end);
  sem_gui_ready = xSemaphoreCreateBinary();
  assert(sem_gui_ready);
#endif

#if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
  ESP_LOGI(TAG, "Turn off LCD backlight");
  gpio_config_t bk_gpio_config = {
      .mode = GPIO_MODE_OUTPUT,
      .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT};
  ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
#endif

  ESP_LOGI(TAG, "Install RGB LCD panel driver");
  esp_lcd_panel_handle_t panel_handle = NULL;
  esp_lcd_rgb_panel_config_t panel_config = {
    .data_width = 16, // RGB565 in parallel mode, thus 16bit in width
    .psram_trans_align = 64,
    .num_fbs = EXAMPLE_LCD_NUM_FB,
#if CONFIG_EXAMPLE_USE_BOUNCE_BUFFER
    .bounce_buffer_size_px = 10 * EXAMPLE_LCD_H_RES,
#endif
    .clk_src = LCD_CLK_SRC_DEFAULT,
    .disp_gpio_num = EXAMPLE_PIN_NUM_DISP_EN,
    .pclk_gpio_num = EXAMPLE_PIN_NUM_PCLK,
    .vsync_gpio_num = EXAMPLE_PIN_NUM_VSYNC,
    .hsync_gpio_num = EXAMPLE_PIN_NUM_HSYNC,
    .de_gpio_num = EXAMPLE_PIN_NUM_DE,
    .data_gpio_nums = {
        EXAMPLE_PIN_NUM_DATA0,
        EXAMPLE_PIN_NUM_DATA1,
        EXAMPLE_PIN_NUM_DATA2,
        EXAMPLE_PIN_NUM_DATA3,
        EXAMPLE_PIN_NUM_DATA4,
        EXAMPLE_PIN_NUM_DATA5,
        EXAMPLE_PIN_NUM_DATA6,
        EXAMPLE_PIN_NUM_DATA7,
        EXAMPLE_PIN_NUM_DATA8,
        EXAMPLE_PIN_NUM_DATA9,
        EXAMPLE_PIN_NUM_DATA10,
        EXAMPLE_PIN_NUM_DATA11,
        EXAMPLE_PIN_NUM_DATA12,
        EXAMPLE_PIN_NUM_DATA13,
        EXAMPLE_PIN_NUM_DATA14,
        EXAMPLE_PIN_NUM_DATA15,
    },
    .timings = {
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .h_res = EXAMPLE_LCD_H_RES,
        .v_res = EXAMPLE_LCD_V_RES,
        // The following parameters should refer to LCD spec
        .hsync_back_porch = 88,
        .hsync_front_porch = 40,
        .hsync_pulse_width = 3,
        .vsync_back_porch = 32,
        .vsync_front_porch = 3,
        .vsync_pulse_width = 3,
        .flags.pclk_active_neg = true,
    },
    .flags.fb_in_psram = 1, // 设为 1 即可，由于 RGB 需要整帧 buffer 用于刷频，只能放在 PSRAM上
                            // .flags.double_fb = 1,                                   // 1 表示使能内部双 buffer（整帧大小），否则仅单 buffer
                            // .flags.refresh_on_demand = 1,                           // 0 表示内部自动刷新，即一帧传输完成后自动开始传输下一帧
                            // 1 表示开启手动刷新，即每帧都需要手动调用 esp_lcd_rgb_panel_refresh()
                            // .bounce_buffer_size_px = 10 * EXAMPLE_LCD_H_RES,            // 设置 bounce buffer 的像素个数（内部会创建两个），见下面详述
  };
  ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));

  ESP_LOGI(TAG, "Register event callbacks");
  esp_lcd_rgb_panel_event_callbacks_t cbs = {
      .on_vsync = example_on_vsync_event,
  };
  ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, &disp_drv));

  ESP_LOGI(TAG, "Initialize RGB LCD panel");
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

#if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
  ESP_LOGI(TAG, "Turn on LCD backlight");
  gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
#endif

  ESP_LOGI(TAG, "Initialize LVGL library");
  lv_init();
  void *buf1 = NULL;
  void *buf2 = NULL;
#if CONFIG_EXAMPLE_DOUBLE_FB
  ESP_LOGI(TAG, "Use frame buffers as LVGL draw buffers");
  ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));
  // initialize LVGL draw buffers
  lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES);
#else
  ESP_LOGI(TAG, "Allocate separate LVGL draw buffers from PSRAM");
  buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 200 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
  assert(buf1);
  buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 200 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
  assert(buf2);
  // initialize LVGL draw buffers
  lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 200);
#endif // CONFIG_EXAMPLE_DOUBLE_FB

  ESP_LOGI(TAG, "Register display driver to LVGL");
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = EXAMPLE_LCD_H_RES;
  disp_drv.ver_res = EXAMPLE_LCD_V_RES;
  disp_drv.flush_cb = example_lvgl_flush_cb;
  disp_drv.draw_buf = &disp_buf;
  disp_drv.user_data = panel_handle;

#if CONFIG_EXAMPLE_DOUBLE_FB
  disp_drv.full_refresh = true; // the full_refresh mode can maintain the synchronization between the two frame buffers
#endif
  lv_disp_drv_register(&disp_drv);
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  ESP_LOGI(TAG, "Install LVGL tick timer");
  // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
  const esp_timer_create_args_t lvgl_tick_timer_args = {
      .callback = &example_increase_lvgl_tick,
      .name = "lvgl_tick"};
  esp_timer_handle_t lvgl_tick_timer = NULL;
  ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

  ESP_LOGI(TAG, "Display LVGL Scatter Chart");

  ui_setup();

  // ESP_ERROR_CHECK(user_init_sdcard());

  // LV_IMG_DECLARE(s220_12fps);
  // lv_obj_t * img;
  // img = lv_gif_create(lv_scr_act());
  // lv_gif_set_src(img, &s220_12fps);
  // lv_obj_align(img, LV_ALIGN_LEFT_MID, 20, 0);

  while (1)
  {
    // raise the task priority of LVGL and/or reduce the handler period can improve the performance
    vTaskDelay(pdMS_TO_TICKS(10));

    // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
    lv_timer_handler();
  }
}
