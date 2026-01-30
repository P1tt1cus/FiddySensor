#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H

#include <stdint.h>

#define HISTORY_SIZE 672  // Event-based: ~28 days with hourly heartbeat

typedef struct {
    uint8_t percent;
    uint32_t timestamp;  // seconds since boot
} history_entry_t;

void moisture_sensor_init(void);
int moisture_sensor_get_raw(void);
int moisture_sensor_get_percent(void);
int moisture_sensor_get_history(history_entry_t *buf, int max_len);
int moisture_sensor_get_history_count(void);
uint32_t moisture_sensor_get_uptime(void);

#endif
