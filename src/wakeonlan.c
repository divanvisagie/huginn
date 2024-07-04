#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define ADDRESS     "tcp://broker.hivemq.com:1883"
#define CLIENTID    "Huginn_pi"
#define TOPIC_SUB   "input_topic"
#define TOPIC_PUB   "output_topic"
#define TOPIC_EXT   "external_processing_topic"
#define QOS         1
#define TIMEOUT     10000L
#define MAC_ADDRESS_LENGTH 6

void send_wake_on_lan(const char *mac_address_str) {
    int sockfd;
    struct sockaddr_in addr;
    unsigned char packet[102];
    unsigned char mac_address[MAC_ADDRESS_LENGTH];

    // Convert MAC address string to byte array
    sscanf(mac_address_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &mac_address[0], &mac_address[1], &mac_address[2],
           &mac_address[3], &mac_address[4], &mac_address[5]);

    // Create the magic packet
    memset(packet, 0xFF, 6); // First 6 bytes are 0xFF
    for (int i = 1; i <= 16; i++) {
        memcpy(&packet[i * MAC_ADDRESS_LENGTH], mac_address, MAC_ADDRESS_LENGTH);
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Enable broadcast
    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Setup the address structure
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9);
    inet_pton(AF_INET, "255.255.255.255", &addr.sin_addr);

    // Send the packet
    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Clean up
    close(sockfd);
    printf("Wake-on-LAN packet sent to %s\n", mac_address_str);
}

