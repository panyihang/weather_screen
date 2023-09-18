#include <stdio.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#include "lvgl_gui.h"

static const char *TAG = "main";

static esp_err_t user_init_sdcard()
{
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
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
    slot_config.clk = 47;
    slot_config.cmd = 21;
    slot_config.d0 = 48;
    slot_config.d1 = 45;
    slot_config.d2 = 13;
    slot_config.d3 = 14;

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

static void print_rtos_status(void *pvParameters)
{
    // char buf[1024];

    while (1)
    {
        size_t size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        size_t size_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        ESP_LOGW(TAG, "%d, %d", size, size_internal);

        // vTaskList(buf);
        // ESP_LOGI(TAG, "\n%s", buf);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

void app_main(void)
{
    // xTaskCreatePinnedToCore(print_rtos_status, "print_rtos_status", 1024 * 4, NULL, 3, NULL, 1);
    ESP_ERROR_CHECK(user_init_sdcard());
    xTaskCreatePinnedToCore(guiTask, "guiTask", 1024 * 6, NULL, 5, NULL, 1);
}
