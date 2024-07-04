#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "wakeonlan.h"

typedef struct {
    char id[50];
    char message[200];
    char models[5][50];
    char strategy[50];
} Message;


int main(int argc, char* argv[]) {
	// Check if MAC address is provided
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <MAC address>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// Send the magic packet
	send_wake_on_lan(argv[1]);

	return EXIT_SUCCESS;
}
