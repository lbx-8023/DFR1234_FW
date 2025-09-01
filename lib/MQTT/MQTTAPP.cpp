#include <stdio.h>
 
#include "mongoose.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mdns.h"
#include "lwip/dns.h"
#include "mqtt_server.h"
#include "mqtt_client.h"
#include "mqtt_publisher.h"
#include "mqtt_subscriber.h"
#include "mqtt_auth.h"
const char *MOUNT_POINT = "/root";
const char *TAG = "MQTTAPP";
  TaskHandle_t broker_hanlde, pub_hanlde, sub_handle;
void mqtt_server_init()
{

	esp_netif_ip_info_t ip_info;
	ESP_ERROR_CHECK(esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info));
	ESP_LOGI(TAG, "ESP32 is STA MODE");
	ESP_LOGI(TAG, "IP Address : " IPSTR, IP2STR(&ip_info.ip));
	ESP_LOGI(TAG, "Subnet Mask: " IPSTR, IP2STR(&ip_info.netmask));
	ESP_LOGI(TAG, "Gateway    : " IPSTR, IP2STR(&ip_info.gw));

	/* Start MQTT Server using tcp transport */
	xTaskCreate(mqtt_server, "BROKER", 1024 * 5, NULL, 7, &broker_hanlde);

	/* Start Subscriber */
	char cparam1[64];
	// sprintf(cparam1, "mqtt://44.232.241.40:1883");
	sprintf(cparam1, "mqtt://" IPSTR ":1883", IP2STR(&ip_info.ip));
	xTaskCreate(mqtt_subscriber, "SUBSCRIBE", 1024 * 5, (void *)cparam1,6, &sub_handle);
	vTaskDelay(10); // You need to wait until the task launch is complete.

	/* Start Publisher */
	char cparam2[64];

	sprintf(cparam2, "mqtt://" IPSTR ":1883", IP2STR(&ip_info.ip));

	xTaskCreate(mqtt_publisher, "PUBLISH", 1024 * 5, (void *)cparam2, 6, &pub_hanlde);
	vTaskDelay(10); // You need to wait until the task launch is complete.
}
void mqtt_server_delet()
{
    if (broker_hanlde) {
        vTaskDelete(broker_hanlde);
        broker_hanlde = NULL;
    }
    if (sub_handle) {
        vTaskDelete(sub_handle);
        sub_handle = NULL;
    }
    if (pub_hanlde) {
        vTaskDelete(pub_hanlde);
        pub_hanlde = NULL;
    }
}
bool mqtt_server_status()
{
	return (broker_hanlde != NULL) && (sub_handle != NULL) && (pub_hanlde != NULL);
}