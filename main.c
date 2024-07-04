#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <mosquitto.h>

#include "wakeonlan.h"
#include "subscriber.h"

const int TIMEOUT = 3000;



int main(int argc, char* argv[]) {
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
