#pragma once
// UDPClient.h

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdint.h>
#include <sys/types.h>
#include <poll.h>


#include "SocketHelpers.h"
#include "RHPPacket.h"

// DON'T TOUCH THESE SETTINGS
#define SERVER "137.112.38.47"
#define PORT 2526
#define BUFSIZE 1024

// DUPLICATE SETTINGS TO DO NOT TOUCH
static const char *SERVERSTR = "137.112.38.47";
static const char *PORTSTR = "2526";

static const char NUM_RETRIES = 5;
static const int TIMEOUT_MS = 200; // milliseconds

extern char msg_out_buffer[RHP_OFFSET_SIZE + RHP_MAX_PAYLOAD_LENGTH];
extern char msg_in_buffer[RHP_MESSAGE_SIZE];

// Useful macros
#define OCTET_COUNT(bits) ((bits / 8) + 1)


// End of UDPClient.h