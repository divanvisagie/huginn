#include <arpa/inet.h>
#include <mosquitto.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "subscriber.h"
#include "wakeonlan.h"

#define MAX_EVENTS 10
#define POLL_TIMEOUT 1000 // Poll timeout in milliseconds

int main(int argc, char *argv[]) {
  struct mosquitto *mosq;
  mosquitto_lib_init();

  mosq = mosquitto_new(NULL, true, NULL);
  if (!mosq) {
    fprintf(stderr, "Error: Out of memory.\n");
    return EXIT_FAILURE;
  }

  mosquitto_connect_callback_set(mosq, on_connect);
  mosquitto_message_callback_set(mosq, on_message);

  int rc;
  rc = mosquitto_connect(mosq, "localhost", 1883, 60);
  if (rc != MOSQ_ERR_SUCCESS) {
    fprintf(stderr, "Unable to connect to the broker: %d\n", rc);
    return EXIT_FAILURE;
  }

  // start the loop
  rc = mosquitto_loop_start(mosq);
  if (rc != MOSQ_ERR_SUCCESS) {
    fprintf(stderr, "Unable to start loop: %s\n", mosquitto_strerror(rc));
    return 1;
  }

  printf("Press Enter to quit...\n");
  getchar();

  mosquitto_loop_stop(mosq, true);

  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();
  return rc;
}
