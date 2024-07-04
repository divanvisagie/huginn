#include <arpa/inet.h>
#include <json-c/json.h>
#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wakeonlan.h"

typedef enum { ExternalOnly, Fallback } Strategy;

#define MAX_MODELS 5
#define MAX_ID_LENGTH 50
#define MAX_MESSAGE_LENGTH 200
#define MAX_MODEL_LENGTH 50
#define MAX_STRATEGY_LENGTH 50

typedef struct {
  char id[MAX_ID_LENGTH];
  char message[MAX_MESSAGE_LENGTH];
  char models[MAX_MODELS][MAX_MODEL_LENGTH];
  char strategy[MAX_STRATEGY_LENGTH];
} Message;

const char *INPUT_TOPIC = "huginn/in";
const char *EXTERNAL_TOPIC = "huginn/defer";

Strategy string_to_strategy(const char *strategy) {
  if (strcmp(strategy, "external_only") == 0) {
    return ExternalOnly;
  } else if (strcmp(strategy, "fallback") == 0) {
    return Fallback;
  } else {
    return Fallback;
  }
}

void parse_json_to_message(const char *json_string, Message *msg) {
    struct json_object *parsed_json;
    struct json_object *id;
    struct json_object *message;
    struct json_object *models;
    struct json_object *model;
    struct json_object *strategy;

    parsed_json = json_tokener_parse(json_string);
    if (parsed_json == NULL) {
        fprintf(stderr, "Error parsing JSON\n");
        return;
    }

    if (json_object_object_get_ex(parsed_json, "id", &id) && json_object_is_type(id, json_type_string)) {
        strncpy(msg->id, json_object_get_string(id), MAX_ID_LENGTH - 1);
    } else {
        fprintf(stderr, "Error: id is not a valid string\n");
    }

    if (json_object_object_get_ex(parsed_json, "message", &message) && json_object_is_type(message, json_type_string)) {
        strncpy(msg->message, json_object_get_string(message), MAX_MESSAGE_LENGTH - 1);
    } else {
        fprintf(stderr, "Error: message is not a valid string\n");
    }

    if (json_object_object_get_ex(parsed_json, "models", &models) && json_object_is_type(models, json_type_array)) {
        int model_index = 0;
        int array_len = json_object_array_length(models);
        for (int i = 0; i < array_len && i < MAX_MODELS; i++) {
            model = json_object_array_get_idx(models, i);
            if (json_object_is_type(model, json_type_string)) {
                strncpy(msg->models[model_index], json_object_get_string(model), MAX_MODEL_LENGTH - 1);
                model_index++;
            } else {
                fprintf(stderr, "Error: model is not a valid string or exceeds max models\n");
            }
        }
    } else {
        fprintf(stderr, "Error: models is not a valid array\n");
    }

    if (json_object_object_get_ex(parsed_json, "strategy", &strategy) && json_object_is_type(strategy, json_type_string)) {
        strncpy(msg->strategy, json_object_get_string(strategy), MAX_STRATEGY_LENGTH - 1);
    } else {
        fprintf(stderr, "Error: strategy is not a valid string\n");
    }

    json_object_put(parsed_json); // Free me
}

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
  int is_remote = getenv("HUGINN_IS_REMOTE") != NULL;
  if (rc == 0) {
    printf("Connected to the broker!\n");

    const char *topic = is_remote ? EXTERNAL_TOPIC : INPUT_TOPIC;
    mosquitto_subscribe(mosq, NULL, topic, 0);
  } else {
    printf("Failed to connect, return code %d\n", rc);
  }
}

/*
 * This is the function that is called when the request should be handled
 * by a remote instance of huginn, it will check if the remote is up and then
 * move the message to the defer topic.
 */
void handle_external(struct mosquitto *mosq, void *obj,
                     const struct mosquitto_message *msg) {
  char *ext_instance = getenv("HUGINN_EXT_INSTANCE");
  if (ext_instance == NULL) {
    fprintf(stderr,
            "Error: Environment variable HUGINN_EXT_INSTANCE not set.\n");
    return;
  }
  send_wake_on_lan(ext_instance);
}

void handle_internal(struct mosquitto *mosq, void *obj,
                     const struct mosquitto_message *msg) {
  printf("Handling internal message\n");
}

void on_message(struct mosquitto *mosq, void *obj,
                const struct mosquitto_message *msg) {
  printf("Received message: %s from topic: %s\n", (char *)msg->payload,
         msg->topic);

  Message *parsed;

  // copy contents of payload to a new string
  char *raw_json = malloc(msg->payloadlen + 1);
  strncpy(raw_json, msg->payload, msg->payloadlen);

  printf("Parsing JSON: %s\n", raw_json);
  parse_json_to_message(raw_json, parsed);

  switch (string_to_strategy(parsed->strategy)) {
  case ExternalOnly:
    handle_external(mosq, obj, msg);
    break;
  case Fallback:
    handle_internal(mosq, obj, msg);
    break;
  }
}
