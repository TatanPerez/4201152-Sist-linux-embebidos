#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "adc_driver.h"
#include "tds.h"
#include "storage.h"

static const char *TAG = "main";

static void tds_task(void *arg)
{
    while (1) {
        float raw = tds_read_raw();
        float ppm = tds_read_ppm();
        ESP_LOGI(TAG, "TDS raw=%.2f ppm=%.2f", raw, ppm);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void console_task(void *arg)
{
    // Simple stdin console. Typing commands followed by Enter.
    char line[64];
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }
        // strip newline
        char *nl = strchr(line, '\n'); if (nl) *nl = '\0';

        if (strcmp(line, "calA") == 0) {
            float raw = tds_read_raw();
            tds_set_calibration_point_A(raw);
            printf("Calibration A saved in RAM: %f\n", raw);
        } else if (strcmp(line, "calB") == 0) {
            float raw = tds_read_raw();
            tds_set_calibration_point_B(raw);
            printf("Calibration B set in RAM: %f\n", raw);
        } else if (strcmp(line, "save") == 0) {
            if (tds_save_calibration() == ESP_OK) printf("Calibration persisted\n");
            else printf("Failed to save calibration\n");
        } else if (strcmp(line, "show") == 0) {
            printf("offset=%f gain=%f\n", tds_get_offset(), tds_get_gain());
        } else if (strlen(line) == 0) {
            // ignore empty
        } else {
            printf("Commands: calA, calB, save, show\n");
        }
    }
}

void app_main(void)
{
    esp_log_level_set("", ESP_LOG_INFO);
    ESP_LOGI(TAG, "Starting TDS project");

    // Init NVS storage
    if (storage_init() != ESP_OK) {
        ESP_LOGW(TAG, "Storage init failed");
    }

    // Init ADC
    if (adc_init() != ESP_OK) {
        ESP_LOGE(TAG, "ADC init failed");
    }

    // Init TDS
    tds_init();

    // Create tasks
    xTaskCreate(tds_task, "tds_task", 4096, NULL, 5, NULL);
    xTaskCreate(console_task, "console_task", 4096, NULL, 5, NULL);
}
