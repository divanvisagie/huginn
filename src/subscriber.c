#include <arpa/inet.h>
#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wakeonlan.h"

typedef struct {
  char id[50];
  char message[200];
  char models[5][50];
  char strategy[50];
} Message;

const char *INPUT_TOPIC = "huginn/in";

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
  if (rc == 0) {
    printf("Connected to the broker!\n");
    // Subscribe to a topic
    mosquitto_subscribe(mosq, NULL, INPUT_TOPIC, 0);
  } else {
    printf("Failed to connect, return code %d\n", rc);
  }
}

void on_message(struct mosquitto *mosq, void *obj,
                const struct mosquitto_message *msg) {
  printf("Received message: %s from topic: %s\n", (char *)msg->payload,
         msg->topic);

  if (strcmp("huginn/in", msg->topic) == 0) {
    printf("Handling internal message\n");
  }
}

void handle_external(struct mosquitto *mosq, void *obj,
                     const struct mosquitto_message *msg) {
  // read external computer mac from environment variable HUGINN_EXT_INSTANCE
  char *ext_instance = getenv("HUGINN_EXT_INSTANCE");
  if (ext_instance == NULL) {
    fprintf(stderr,
            "Error: Environment variable HUGINN_EXT_INSTANCE not set.\n");
    return;
  }
  send_wake_on_lan(ext_instance);
}
