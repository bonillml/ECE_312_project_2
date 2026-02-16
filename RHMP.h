#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define BIT_OFFSET_COMM_ID 0
#define COMM_ID_MASK 0x00003FFF
#define BIT_OFFSET_TYPE 14
#define TYPE_MASK 0x000FC000
#define BIT_OFFSET_LENGTH 20
#define LENGTH_MASK 0xFFF00000
#define BIT_OFFSET_LENGTH 20
#define RMHMP_MSG_TYPE_RESERVED 0
#define RMHMP_MSG_TYPE_MSG_REQUEST 4
#define RMHMP_MSG_TYPE_MSG_RESPONSE 6
#define RMHMP_MSG_TYPE_ID_REQUEST 16
#define RMHMP_MSG_TYPE_ID_RESPONSE 24


#define COMM_ID 0x312

typedef struct RHMPHeaderRaw {
    uint32_t commID_Type_and_Length; // 14 bits for commID, 6 bits for type, 12 bits for length
} __attribute__((packed)) RHMPHeaderRaw;
typedef struct RHMPFields{
    uint16_t commID;
    uint8_t type;
    uint16_t length;
} RHMPFields;

#define RHMP_HEADER_SIZE sizeof(struct RHMPHeaderRaw)
#define RHMP_MAX_PAYLOAD_LENGTH 4095
#define RHMP_MIN_PAYLOAD_LENGTH 0
#define RHMP_MAX_MESSAGE_SIZE (RHMP_HEADER_SIZE + RHMP_MAX_PAYLOAD_LENGTH)
#define RHMP_MIN_MESSAGE_SIZE (RHMP_HEADER_SIZE + RHMP_MIN_PAYLOAD_LENGTH)

int writeRHMPmsgToBuffer(struct RHMPFields *msg, char *msgOutBuffer, uint16_t bufferLength);
int readRHMPMessageFromBuffer(char *msg, struct RHMPFields *fields, char *msgInBuffer, uint16_t lengthOfMsgInBuffer);

