#ifndef __MQTTSUB_H_
#define __MQTTSUB_H_
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "mongoose.h"
#ifdef __cplusplus
extern "C" {
#endif
#define MAX_MQTT_PAYLOAD_LEN 1024
typedef struct {
    char topic[32];
    char message[MAX_MQTT_PAYLOAD_LEN];
} MQTTMessage;

void mqtt_subscriber(void *pvParameters);
#ifdef __cplusplus
}
#endif
#endif