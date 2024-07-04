#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <mosquitto.h>

#include "wakeonlan.h"

const int TIMEOUT = 3000;

typedef struct {
    char id[50];
    char message[200];
    char models[5][50];
    char strategy[50];
} Message;

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    if (rc == 0) {
        printf("Connected to the broker!\n");
        // Subscribe to a topic
        mosquitto_subscribe(mosq, NULL, "test/topic", 0);
    } else {
        printf("Failed to connect, return code %d\n", rc);
    }
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    printf("Received message: %s from topic: %s\n", (char *)msg->payload, msg->topic);
}

int main(int argc, char* argv[]) {
	// Check if MAC address is provided
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <MAC address>\n", argv[0]);
		return EXIT_FAILURE;
	}

	struct mosquitto *mosq;
	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, NULL);
	if (!mosq) {
		fprintf(stderr, "Error: Out of memory.\n");
		return EXIT_FAILURE;
	}
	
	// Set up the callbacks
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);

	int rc;
	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	if (rc != MOSQ_ERR_SUCCESS) {
		fprintf(stderr, "Unable to connect to the broker: %d\n", rc);
		return EXIT_FAILURE;
	}

	// start the network loop
	rc = mosquitto_loop_forever(mosq, TIMEOUT, 1);

	// // Send the magic packet
	// send_wake_on_lan(argv[1]);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	return rc;
}
