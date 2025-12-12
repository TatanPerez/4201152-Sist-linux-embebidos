/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/*  WiFi softAP & station Example */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"

#include "pump_driver.h"
#include "ultrasonic_driver.h"
#include "tds_driver.h"
#include "net_manager.h"

/* Peripheral pins */
#define PUMP_GPIO_PIN GPIO_NUM_12
#define ULTRASONIC_TRIG_GPIO GPIO_NUM_5
#define ULTRASONIC_ECHO_GPIO GPIO_NUM_18
#define TDS_ADC_CHANNEL ADC_CHANNEL_6

#define PUMP_QUEUE_LEN 4
#define TELEMETRY_QUEUE_LEN 8
#define TDS_CMD_QUEUE_LEN 4

static const char *TAG_APP = "Cisterna";

/* MQTT topics */
static const char *TOPIC_PUMP_CMD = "cisterna/bomba/set";
static const char *TOPIC_PUMP_STATE = "cisterna/bomba/state";
static const char *TOPIC_ULTRASONIC = "cisterna/ultrasonido";
static const char *TOPIC_TDS = "cisterna/tds";
static const char *TOPIC_TDS_CAL_CMD = "cisterna/tds/cal";
static const char *TOPIC_TDS_CAL_ACK = "cisterna/tds/cal/ack";

typedef struct {
    esp_mqtt_client_handle_t mqtt;
    QueueHandle_t pump_cmd_queue;
    QueueHandle_t telemetry_queue;
    QueueHandle_t tds_cmd_queue;
} app_context_t;

typedef struct {
    bool turn_on;
} pump_cmd_msg_t;

typedef struct {
    char topic[48];
    float value;
} telemetry_msg_t;

typedef enum {
    TDS_CMD_CAL_A,
    TDS_CMD_CAL_B,
    TDS_CMD_SAVE,
    TDS_CMD_LOAD,
} tds_cmd_type_t;

typedef struct {
    tds_cmd_type_t type;
} tds_cmd_msg_t;

static void pump_publish_state(app_context_t *app);

static void telemetry_publish_task(void *pvParameters)
{
    app_context_t *app = (app_context_t *)pvParameters;
    telemetry_msg_t msg;
    char payload[32];
    while (true) {
        if (xQueueReceive(app->telemetry_queue, &msg, portMAX_DELAY) == pdTRUE && app->mqtt) {
            snprintf(payload, sizeof(payload), "%.2f", msg.value);
            esp_mqtt_client_publish(app->mqtt, msg.topic, payload, 0, 1, 0);
        }
    }
}

static void pump_publish_state(app_context_t *app)
{
    if (!app || !app->mqtt) {
        return;
    }
    esp_mqtt_client_publish(app->mqtt, TOPIC_PUMP_STATE,
                            pump_driver_get_state() ? "ON" : "OFF",
                            0, 1, 1);
}

static void tds_cal_ack(app_context_t *app, const char *msg)
{
    if (app && app->mqtt && msg) {
        esp_mqtt_client_publish(app->mqtt, TOPIC_TDS_CAL_ACK, msg, 0, 1, 0);
    }
}

static void tds_cal_task(void *pvParameters)
{
    app_context_t *app = (app_context_t *)pvParameters;
    tds_cmd_msg_t cmd;
    char ack[96];
    while (true) {
        if (xQueueReceive(app->tds_cmd_queue, &cmd, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        switch (cmd.type) {
        case TDS_CMD_CAL_A: {
            float raw = tds_driver_read_raw();
            if (raw >= 0) {
                tds_set_calibration_point_A(raw);
                snprintf(ack, sizeof(ack), "CAL_A raw=%.2f", raw);
                ESP_LOGI(TAG_APP, "TDS CAL_A raw=%.2f offset=%.2f gain=%.4f", raw, tds_get_offset(), tds_get_gain());
            } else {
                snprintf(ack, sizeof(ack), "CAL_A failed");
                ESP_LOGW(TAG_APP, "TDS CAL_A failed");
            }
            tds_cal_ack(app, ack);
            break;
        }
        case TDS_CMD_CAL_B: {
            float raw = tds_driver_read_raw();
            if (raw >= 0) {
                tds_set_calibration_point_B(raw);
                snprintf(ack, sizeof(ack), "CAL_B raw=%.2f gain=%.4f", raw, tds_get_gain());
                ESP_LOGI(TAG_APP, "TDS CAL_B raw=%.2f offset=%.2f gain=%.4f", raw, tds_get_offset(), tds_get_gain());
            } else {
                snprintf(ack, sizeof(ack), "CAL_B failed");
                ESP_LOGW(TAG_APP, "TDS CAL_B failed");
            }
            tds_cal_ack(app, ack);
            break;
        }
        case TDS_CMD_SAVE: {
            esp_err_t r = tds_save_calibration();
            snprintf(ack, sizeof(ack), "SAVE %s", r == ESP_OK ? "OK" : "ERR");
            ESP_LOGI(TAG_APP, "TDS SAVE %s offset=%.2f gain=%.4f", r == ESP_OK ? "OK" : "ERR", tds_get_offset(), tds_get_gain());
            tds_cal_ack(app, ack);
            break;
        }
        case TDS_CMD_LOAD: {
            esp_err_t r = tds_load_calibration();
            snprintf(ack, sizeof(ack), "LOAD %s offset=%.2f gain=%.4f",
                     r == ESP_OK ? "OK" : "DEF", tds_get_offset(), tds_get_gain());
            ESP_LOGI(TAG_APP, "TDS LOAD %s offset=%.2f gain=%.4f",
                     r == ESP_OK ? "OK" : "DEF", tds_get_offset(), tds_get_gain());
            tds_cal_ack(app, ack);
            break;
        }
        default:
            break;
        }
    }
}

static void pump_cmd_task(void *pvParameters)
{
    app_context_t *app = (app_context_t *)pvParameters;
    pump_cmd_msg_t cmd;
    while (true) {
        if (xQueueReceive(app->pump_cmd_queue, &cmd, portMAX_DELAY) == pdTRUE) {
            pump_driver_set_state(cmd.turn_on);
            pump_publish_state(app);
            ESP_LOGI(TAG_APP, "Pump command -> %s", cmd.turn_on ? "ON" : "OFF");
        }
    }
}

static void enqueue_telemetry(app_context_t *app, const char *topic, float value)
{
    if (!app || !app->telemetry_queue) {
        return;
    }
    telemetry_msg_t msg = {0};
    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    msg.value = value;
    xQueueSend(app->telemetry_queue, &msg, 0);
}

static void sensor_task(void *pvParameters)
{
    app_context_t *app = (app_context_t *)pvParameters;
    while (true) {
        float distance = ultrasonic_driver_read_cm();
        if (distance > 0) {
            enqueue_telemetry(app, TOPIC_ULTRASONIC, distance);
            ESP_LOGI(TAG_APP, "Ultrasonic distance: %.2f cm", distance);
        } else {
            ESP_LOGW(TAG_APP, "Ultrasonic read timeout");
        }

        float tds = tds_driver_read_ppm();
        enqueue_telemetry(app, TOPIC_TDS, tds);
        ESP_LOGI(TAG_APP, "TDS reading: %.2f", tds);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    app_context_t *app = (app_context_t *)handler_args;
    esp_mqtt_event_handle_t event = event_data;
    switch (event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG_APP, "Connected to MQTT broker");
        esp_mqtt_client_subscribe(event->client, TOPIC_PUMP_CMD, 1);
        if (app && app->pump_cmd_queue) {
            pump_cmd_msg_t cmd = {.turn_on = false};
            xQueueSend(app->pump_cmd_queue, &cmd, 0);
        }
        break;
    case MQTT_EVENT_DATA: {
        char topic[64] = {0};
        char data[16] = {0};
        size_t topic_len = event->topic_len < (sizeof(topic) - 1) ? event->topic_len : (sizeof(topic) - 1);
        size_t data_len = event->data_len < (sizeof(data) - 1) ? event->data_len : (sizeof(data) - 1);
        memcpy(topic, event->topic, topic_len);
        memcpy(data, event->data, data_len);

        if (strcmp(topic, TOPIC_PUMP_CMD) == 0) {
            bool turn_on = (strncasecmp(data, "on", 2) == 0) || (data[0] == '1');
            if (app && app->pump_cmd_queue) {
                pump_cmd_msg_t cmd = {.turn_on = turn_on};
                xQueueSend(app->pump_cmd_queue, &cmd, 0);
            }
        } else if (strcmp(topic, TOPIC_TDS_CAL_CMD) == 0 && app && app->tds_cmd_queue) {
            tds_cmd_msg_t cmd;
            if (strncasecmp(data, "calA", 4) == 0) {
                cmd.type = TDS_CMD_CAL_A;
                xQueueSend(app->tds_cmd_queue, &cmd, 0);
            } else if (strncasecmp(data, "calB", 4) == 0) {
                cmd.type = TDS_CMD_CAL_B;
                xQueueSend(app->tds_cmd_queue, &cmd, 0);
            } else if (strncasecmp(data, "save", 4) == 0) {
                cmd.type = TDS_CMD_SAVE;
                xQueueSend(app->tds_cmd_queue, &cmd, 0);
            } else if (strncasecmp(data, "load", 4) == 0) {
                cmd.type = TDS_CMD_LOAD;
                xQueueSend(app->tds_cmd_queue, &cmd, 0);
            } else {
                tds_cal_ack(app, "Unknown cmd");
            }
        }
        break;
    }
    default:
        break;
    }
}

void app_main(void)
{
    net_manager_context_t net_ctx = {0};
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    app_context_t *app_ctx = calloc(1, sizeof(app_context_t));
    if (!app_ctx) {
        ESP_LOGE(TAG_APP, "Failed to allocate app context");
        return;
    }
    app_ctx->pump_cmd_queue = xQueueCreate(PUMP_QUEUE_LEN, sizeof(pump_cmd_msg_t));
    app_ctx->telemetry_queue = xQueueCreate(TELEMETRY_QUEUE_LEN, sizeof(telemetry_msg_t));
    if (!app_ctx->pump_cmd_queue || !app_ctx->telemetry_queue) {
        ESP_LOGE(TAG_APP, "Failed to create queues");
        return;
    }
    app_ctx->tds_cmd_queue = xQueueCreate(TDS_CMD_QUEUE_LEN, sizeof(tds_cmd_msg_t));
    if (!app_ctx->tds_cmd_queue) {
        ESP_LOGE(TAG_APP, "Failed to create TDS queue");
        return;
    }

    ESP_ERROR_CHECK(net_manager_start(&net_ctx, mqtt_event_handler, app_ctx));
    app_ctx->mqtt = net_ctx.mqtt_client;

    ESP_ERROR_CHECK(pump_driver_init(PUMP_GPIO_PIN));
    ESP_ERROR_CHECK(ultrasonic_driver_init(ULTRASONIC_TRIG_GPIO, ULTRASONIC_ECHO_GPIO));
    ESP_ERROR_CHECK(tds_driver_init(TDS_ADC_CHANNEL));

    xTaskCreate(sensor_task, "sensor_task", 4096, app_ctx, 5, NULL);
    xTaskCreate(pump_cmd_task, "pump_cmd_task", 3072, app_ctx, 5, NULL);
    xTaskCreate(telemetry_publish_task, "telemetry_publish_task", 3072, app_ctx, 5, NULL);
    xTaskCreate(tds_cal_task, "tds_cal_task", 3072, app_ctx, 5, NULL);

    vTaskDelay(portMAX_DELAY);
}
