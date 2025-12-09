#include "storage.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "storage";

esp_err_t storage_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "NVS initialized");
    } else {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t storage_save_float(const char *key, float value)
{
    esp_err_t ret;
    nvs_handle_t handle;
    ret = nvs_open("tds_storage", NVS_READWRITE, &handle);
    if (ret != ESP_OK) return ret;
    uint32_t raw;
    memcpy(&raw, &value, sizeof(float));
    ret = nvs_set_u32(handle, key, raw);
    if (ret == ESP_OK) ret = nvs_commit(handle);
    nvs_close(handle);
    if (ret == ESP_OK) ESP_LOGI(TAG, "Saved [%s]=%f", key, value);
    else ESP_LOGE(TAG, "Failed to save [%s]: %s", key, esp_err_to_name(ret));
    return ret;
}

esp_err_t storage_load_float(const char *key, float *value)
{
    esp_err_t ret;
    nvs_handle_t handle;
    ret = nvs_open("tds_storage", NVS_READONLY, &handle);
    if (ret != ESP_OK) return ret;
    uint32_t raw = 0;
    ret = nvs_get_u32(handle, key, &raw);
    if (ret == ESP_OK) {
        memcpy(value, &raw, sizeof(float));
        ESP_LOGI(TAG, "Loaded [%s]=%f", key, *value);
    } else {
        ESP_LOGW(TAG, "Key [%s] not found: %s", key, esp_err_to_name(ret));
    }
    nvs_close(handle);
    return ret;
}
