#ifndef __MQTTPUBLISHER_H_
#define __MQTTPUBLISHER_H_
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
extern "C"
{
#endif
	void mqtt_publisher(void *pvParameters);

	void mqtt_subscriber(void *pvParameters);
#ifdef __cplusplus
}
#endif
#endif