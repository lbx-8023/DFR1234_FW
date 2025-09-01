#ifndef MAIN_MQTT_SERVER_H_
#define MAIN_MQTT_SERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mongoose.h"

// A list of client, held in memory
struct client {
  struct client *next;
  struct mg_connection *c;
  struct mg_str cid;
};

// A list of subscription, held in memory
struct sub {
  struct sub *next;
  struct mg_connection *c;
  struct mg_str topic;
  uint8_t qos;
};

// A list of will topic & message, held in memory
struct will {
  struct will *next;
  struct mg_connection *c;
  struct mg_str topic;
  struct mg_str payload;
  uint8_t qos;
  uint8_t retain;
};

void mqtt_server(void *pvParameters);
#ifdef __cplusplus
}
#endif
#endif /* MAIN_MQTT_SERVER_H_ */
