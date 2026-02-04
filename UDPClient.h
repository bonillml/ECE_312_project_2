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

#include "SocketHelpers.h"

// DON'T TOUCH THESE SETTINGS
#define SERVER "137.112.38.47"
#define PORT 2526
#define BUFSIZE 1024

// DUPLICATE SETTINGS TO DO NOT TOUCH
static const char *SERVERSTR = "137.112.38.47";
static const char *PORTSTR = "2526";

// RHP Packet Settings (touchable)
static const int DEFAULT_NUM_OCTETS = 9;
static const uint8_t RHP_VERSION = 12;
static const uint16_t SOURCE_PORT = 3285;
static const uint16_t CTRL_MSG_PORT = 0x1874;
static const uint16_t RHMP_MSG_PORT = 0x0ECE;

// Message settings (touchable)
#define MESSAGE_SIZE 256
char msg_out_buffer[MESSAGE_SIZE];
char msg_in_buffer[MESSAGE_SIZE];

// Useful macros
#define OCTET_COUNT(bits) ((bits / 8) + 1)

// Method signatures
struct RHP *createRHPPacket(char msg[], uint8_t type);
void computeChecksum16(struct RHP *packet);

struct RHPHeader
{
    uint8_t version;   // Version number of RHP protocol.
    uint16_t srcPort;  // RHP port of source.
    uint16_t destPort; // Destination RHP port.

    // Keep length values in range 0 - 4095
    // Keep type values in range 0 - 15
    // 0 = Control message (ASCII Strings)
    //      (Use 0x1874 for the dstPort)
    // 4 = RHMP message
    //      (Use 0xECE for the dstPort)
    // The dstPort will be automatically assigned given the type.
    //
    // The length and type are combined to form two octets.
    uint16_t length_and_type; // Length of payload (bytes) & RHP message type (payload protocol)
};



struct RHP
{
    uint8_t version;   // Version number of RHP protocol.
    uint16_t srcPort;  // RHP port of source.
    uint16_t destPort; // Destination RHP port.

    // Keep length values in range 0 - 4095
    // Keep type values in range 0 - 15
    // 0 = Control message (ASCII Strings)
    //      (Use 0x1874 for the dstPort)
    // 4 = RHMP message
    //      (Use 0xECE for the dstPort)
    // The dstPort will be automatically assigned given the type.
    //
    // The length and type are combined to form two octets.
    uint16_t length_and_type; // Length of payload (bytes) & RHP message type (payload protocol)

    // 0 = RHP packet won't have a buffer.
    // 1 = RHP packet will have a buffer.
    uint8_t buffer; // Optional 8-bit buffer of zeros to ensure even number of octects in packet.

    // Has a variable size.
    // Can max at 4096 bytes
    char payload[4096]; // RHP SDU (depedent on type)

    uint16_t checksum; // 16-bit internet checksum.

    // Tracks the total number of octets in this packet.
    uint16_t totalOctetCount;
};


// End of UDPClient.h