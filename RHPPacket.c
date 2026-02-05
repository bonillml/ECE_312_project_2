

#include "RHPPacket.h"

// Assembles an RHP packet into the provided buffer, including checksum
int createRHPPacketFromArray(char *msg, uint8_t type, char *packetOutBuffer, uint16_t lengthOfMsg, struct RHPHeader *header)
{

    header->version = RHP_VERSION;
    header->srcPort = RHP_SOURCE_PORT;

    type &= 0x0F; // Ensure type is 4 bits
    switch (type)
    {
    case RHP_TYPE_CTRL_MSG:
        header->destPort = RHP_PORT_CTRL_MSG;
        break;
    case RHP_TYPE_RHMP_MSG:
        header->destPort = RHP_PORT_RHMP_MSG;
        break;
    default:
        return -1; // Invalid type
    }

    lengthOfMsg &= 0x0FFF; // Convert to bytes

    if (lengthOfMsg < 0 || lengthOfMsg > RHP_MAX_PAYLOAD_LENGTH)
    {
        return -1; // Invalid length
    }

    header->length_and_type = ((0x0FFF & lengthOfMsg) << 4) | (type & 0x0F);

    size_t oddOffset = (lengthOfMsg + 1) % 2; // offset is 1 if payload is even, 0 if odd, to ensure even total octet count
    memcpy(packetOutBuffer, header, sizeof(struct RHPHeader));
    memset(packetOutBuffer + sizeof(struct RHPHeader), 0, oddOffset); // Clear buffer area if needed
    memcpy(packetOutBuffer + sizeof(struct RHPHeader) + oddOffset, msg, lengthOfMsg);

    uint16_t checkSum = calculateChecksum(packetOutBuffer, sizeof(struct RHPHeader) + oddOffset + lengthOfMsg);

    // Append checksum to packet buffer
    memcpy(packetOutBuffer + sizeof(struct RHPHeader) + oddOffset + lengthOfMsg, &checkSum, sizeof(checkSum));

    return sizeof(struct RHPHeader) + oddOffset + lengthOfMsg + sizeof(checkSum);
}

struct RHP *createRHPPacket(char msg[], uint8_t type)
{

    struct RHP *packet = malloc(sizeof(struct RHP));

    uint8_t totalOctetCount = DEFAULT_NUM_OCTETS;

    // Assign the RHP version.
    packet->version = RHP_VERSION;

    // Assign the source port.
    packet->srcPort = RHP_SOURCE_PORT;

    // Check the type and length validity
    uint16_t length = (sizeof(char) * (strlen(msg) + 1));
    if ((!type || type == 4) && length < 4096)
    {

        // Valid length, so increment octet count.
        totalOctetCount += length;

        // Populate the length and type octets
        packet->length_and_type = ((0x3 | type) << 3) | (0x0FFF & length);

        // Assign the destination port.
        if (type == 4)
            packet->destPort = 0x0ECE;
        else
            packet->destPort = 0x1874;
    }
    else
        return NULL;

    // If there an odd number of octets, add buffer.
    if ((totalOctetCount % 2))
    {
        packet->buffer = 1;
        totalOctetCount++;
    }

    // Set total octet count
    packet->totalOctetCount = totalOctetCount;
    printf("> Total packet octet: %d\n", totalOctetCount);

    // Compute the packet's checksum
    // uint16_t checksum = computeChecksum16(packet);
    appendChecksum(packet);
    return packet;
}

void appendChecksum(struct RHP *packet)
{

    uint32_t sum = 0x0000;

    // Add the version and lower half of source port
    sum += ((0x0F & packet->srcPort) << 8) | (0x0F & (uint16_t)packet->version);

    // Add the high half of source port and the lower half of destination port
    sum += ((0xF0 & packet->srcPort)) | (0x0F & (packet->destPort));
}

uint16_t calculateChecksum(char *msg, ssize_t length)
{
    uint32_t sum = 0x0000;
    if (length <= 0)
    {
        perror("length is out of bounds for checksum calculation");
        return 0;
    }
    if (length % 2 != 0)
    {
        perror("length of message odd for checksum calculation, must be even");
        return 0; // pad to even number of bytes
    }

    for (int i = 0; i < length; i += 2) // process two bytes at a time
    {
        uint16_t word = msg[i] << 8;
        if (i + 1 < length)
        {
            word |= msg[i + 1];
        }
        sum += word;
    }
    if (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16); // handle carry
    }
    return (uint16_t)(~sum & 0xFFFF); // One's complement
}
