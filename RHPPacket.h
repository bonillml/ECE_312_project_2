#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <poll.h>

#include "SocketHelpers.h"

// RHP Packet Settings (touchable)
#define DEFAULT_NUM_OCTETS 9
#define RHP_VERSION 12
#define RHP_SOURCE_PORT 3285
#define RHP_PORT_CTRL_MSG 0x1874
#define RHP_PORT_RHMP_MSG 0x0ECE
#define RHP_TYPE_CTRL_MSG 0
#define RHP_TYPE_RHMP_MSG 4

typedef struct RHPHeader
{
    uint8_t version;   // Version number of RHP protocol.
    uint16_t srcPort;  // RHP port of source.
    uint16_t destPort; // Destination RHP port.

    // Keep length values in range 0 - 4095 //why 4095?, is it because theres null termination??
    // Keep type values in range 0 - 15
    // 0 = Control message (ASCII Strings)
    //      (Use 0x1874 for the dstPort)
    // 4 = RHMP message
    //      (Use 0xECE for the dstPort)
    // The dstPort will be automatically assigned given the type.
    //
    // The length and type are combined to form two octets.
    uint16_t length_and_type; // Length of payload (bytes) & RHP message type (payload protocol)
} __attribute__((packed)) RHPHeader;

struct RHPFields
{
    uint8_t version;   // Version number of RHP protocol.
    uint16_t srcPort;
    uint16_t destPort;
    uint16_t length;
    uint8_t type;
    uint16_t checkSum;
} ;


// Message settings (touchable)
#define RHP_HEADER_SIZE (sizeof(struct RHPHeader))
#define RHP_OFFSET_SIZE (sizeof(uint8_t))
#define RHP_MAX_PAYLOAD_LENGTH 4095 // defined in spec
#define RHP_CHECKSUM_LENGTH (sizeof(uint16_t))
#define RHP_MAX_MESSAGE_SIZE (RHP_HEADER_SIZE + RHP_OFFSET_SIZE + RHP_MAX_PAYLOAD_LENGTH + RHP_CHECKSUM_LENGTH)
#define RHP_MIN_MESSAGE_SIZE (RHP_HEADER_SIZE + sizeof(uint8_t) + RHP_CHECKSUM_LENGTH) // minimum size with 0 byte payload and 1 byte buffer
#define PACKET_INTEGRITY_CHECK_FAILED -1
#define PACKET_INTEGRITY_CHECK_FAILED_CHECKSUM -2

// Method signatures
struct RHP *createRHPPacket(char msg[], uint8_t type);

void appendChecksum(struct RHP *packet);
uint16_t calculateChecksum(const char *msg, ssize_t length);

int createRHPPacketFromArray(char *msg, uint8_t type, char packetOutBuffer[], uint16_t lengthOfMsg);
int sendPacketGetAck(int socketfd, struct addrinfo *serverAddr, char *packetOutBuffer, size_t packetSize, char *packetInBuffer, size_t maxPacketInSize, int timeoutMs, int maxRetries);

int packetIntgrityCheck(const char *packetBuffer, size_t packetSize);

int printRHPPacketInfo(const char *packetBuffer, size_t packetSize);
int printRHPPacketPayload(const char *packetBuffer, bool asString, size_t packetSize);
int isPacketPayloadNullTerminated(const char *packetBuffer, size_t packetSize);

int parseRHPInfoFromBuffer(const char *packetBuffer, size_t packetSize, struct RHPFields *fields);


void checkSumTester(void);
