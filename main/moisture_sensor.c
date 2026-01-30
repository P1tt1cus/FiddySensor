#include "moisture_sensor.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// GPIO 34 = ADC1_CH6 (works with WiFi, unlike ADC2 pins)
#define MOISTURE_ADC_CHANNEL    ADC_CHANNEL_6
#define MOISTURE_ADC_ATTEN      ADC_ATTEN_DB_12
#define CHECK_INTERVAL_MS       5000        // Check every 5 seconds
#define CHANGE_THRESHOLD        5           // Record if 5% change
#define MAX_SILENT_MS           (60*60*1000) // Record at least once per hour

static const char *TAG = "moisture";
static adc_oneshot_unit_handle_t s_adc_handle = NULL;

// Calibration values
#define RAW_DRY     50
#define RAW_WET     2500

// History buffer (circular) with thread safety
static history_entry_t s_history[HISTORY_SIZE];
static int s_history_head = 0;
static int s_history_count = 0;
static SemaphoreHandle_t s_history_mutex = NULL;

static uint32_t get_timestamp(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000000);  // microseconds to seconds
}

static void add_to_history(uint8_t percent)
{
    xSemaphoreTake(s_history_mutex, portMAX_DELAY);
    s_history[s_history_head].percent = percent;
    s_history[s_history_head].timestamp = get_timestamp();
    s_history_head = (s_history_head + 1) % HISTORY_SIZE;
    if (s_history_count < HISTORY_SIZE) s_history_count++;
    xSemaphoreGive(s_history_mutex);
}

static void history_task(void *arg)
{
    int last_recorded = moisture_sensor_get_percent();
    int silent_time = 0;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(CHECK_INTERVAL_MS));
        silent_time += CHECK_INTERVAL_MS;

        int current = moisture_sensor_get_percent();
        int diff = (last_recorded < 0) ? 100 : abs(current - last_recorded);

        // Record if: first reading, significant change, or been too quiet
        if (last_recorded < 0 || diff >= CHANGE_THRESHOLD || silent_time >= MAX_SILENT_MS) {
            add_to_history((uint8_t)current);
            ESP_LOGI(TAG, "Recorded %d%% (change:%d%%) [%d events]", current, diff, s_history_count);
            last_recorded = current;
            silent_time = 0;
        }
    }
}

void moisture_sensor_init(void)
{
    s_history_mutex = xSemaphoreCreateMutex();

    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&unit_cfg, &s_adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = MOISTURE_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_oneshot_config_channel(s_adc_handle, MOISTURE_ADC_CHANNEL, &chan_cfg);

    // Record first reading immediately
    add_to_history((uint8_t)moisture_sensor_get_percent());

    // Start history recording task
    xTaskCreate(history_task, "history", 2048, NULL, 3, NULL);

    ESP_LOGI(TAG, "Moisture sensor ready on GPIO 34");
}

int moisture_sensor_get_raw(void)
{
    int raw = 0;
    adc_oneshot_read(s_adc_handle, MOISTURE_ADC_CHANNEL, &raw);
    return raw;
}

int moisture_sensor_get_percent(void)
{
    int raw = moisture_sensor_get_raw();
    int percent = (raw - RAW_DRY) * 100 / (RAW_WET - RAW_DRY);
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    return percent;
}

int moisture_sensor_get_history(history_entry_t *buf, int max_len)
{
    xSemaphoreTake(s_history_mutex, portMAX_DELAY);
    int count = (s_history_count < max_len) ? s_history_count : max_len;
    int start = (s_history_head - count + HISTORY_SIZE) % HISTORY_SIZE;

    for (int i = 0; i < count; i++) {
        buf[i] = s_history[(start + i) % HISTORY_SIZE];
    }
    xSemaphoreGive(s_history_mutex);
    return count;
}

int moisture_sensor_get_history_count(void)
{
    xSemaphoreTake(s_history_mutex, portMAX_DELAY);
    int count = s_history_count;
    xSemaphoreGive(s_history_mutex);
    return count;
}

uint32_t moisture_sensor_get_uptime(void)
{
    return get_timestamp();
}
