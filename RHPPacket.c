#include "RHPPacket.h"

// Assembles an RHP packet into the provided buffer, including checksum
int createRHPPacketFromArray(char *msg, uint8_t type, char *packetOutBuffer, uint16_t lengthOfMsg, struct RHPHeader *header)
{

    header->version = RHP_VERSION;
    printf("> RHP Version: %d\n", header->version);
    header->srcPort = htons(RHP_SOURCE_PORT);

    type &= 0x0F; // Ensure type is 4 bits
    switch (type)
    {
    case RHP_TYPE_CTRL_MSG:
        header->destPort = htons(RHP_PORT_CTRL_MSG);
        break;
    case RHP_TYPE_RHMP_MSG:
        header->destPort = htons(RHP_PORT_RHMP_MSG);
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
    header->length_and_type = htons(header->length_and_type);

    size_t oddOffset = (lengthOfMsg + 1) % 2; // offset is 1 if payload is even, 0 if odd, to ensure even total octet count
    memcpy(packetOutBuffer, header, sizeof(struct RHPHeader));
    memset(packetOutBuffer + sizeof(struct RHPHeader), 0, oddOffset); // Clear buffer area if needed
    memcpy(packetOutBuffer + sizeof(struct RHPHeader) + oddOffset, msg, lengthOfMsg);

    uint16_t checkSum = calculateChecksum(packetOutBuffer, sizeof(struct RHPHeader) + oddOffset + lengthOfMsg);
    uint16_t checkSumNetworkOrder = htons(checkSum);

    // Append checksum to packet buffer
    memcpy(packetOutBuffer + sizeof(struct RHPHeader) + oddOffset + lengthOfMsg, &checkSumNetworkOrder, sizeof(checkSumNetworkOrder));

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


void printRHPPacket(const char *packetBuffer, bool isNetworkOrder)
{
    const uint8_t *rawPacket = (const uint8_t *)packetBuffer;
    struct RHPHeader packetHeader;
    memcpy(&packetHeader, rawPacket, sizeof(struct RHPHeader));

    uint16_t payloadLengthValue;
    uint8_t packetType;

    if(isNetworkOrder)
    {
        payloadLengthValue = (ntohs(packetHeader.length_and_type) >> 4) & 0x0FFF;
        packetType = ntohs(packetHeader.length_and_type) & 0x000F;
        // Convert fields from network to Host byte order
        packetHeader.srcPort = ntohs(packetHeader.srcPort);
        packetHeader.destPort = ntohs(packetHeader.destPort);
        packetHeader.length_and_type = ntohs(packetHeader.length_and_type);
        
    }
    else //its in host order
    {
        payloadLengthValue = (packetHeader.length_and_type >> 4) & 0x0FFF;
        packetType = packetHeader.length_and_type & 0x000F;
    }

    

    printf("RHP Packet:\n");
    printf("Byte order: %s\n", isNetworkOrder ? "Network Order" : "Host Order");
    
    printf(" Version: %d\n", packetHeader.version);
    printf("  Value : %u\n", packetHeader.version);
    printf("  Raw   : %02X\n", rawPacket[0]);

    printf(" Source Port: %d\n", packetHeader.srcPort);
    printf("  Value : %u\n", packetHeader.srcPort);
    printf("  Raw   : %02X %02X\n", rawPacket[1], rawPacket[2]);

    printf(" Destination Port: %d\n", packetHeader.destPort);
    printf("  Value : %u\n", packetHeader.destPort);
    printf("  Raw   : %02X %02X\n", rawPacket[3], rawPacket[4]);

    printf(" Length and Type: %d\n", packetHeader.length_and_type);
    printf("  Value (combined): %u\n", packetHeader.length_and_type);
    printf("  Raw             : %02X %02X\n", rawPacket[5], rawPacket[6]);
    printf("  Decoded Length  : %u bytes\n", payloadLengthValue);
    printf("  Decoded Type    : %u (", packetType);
    switch (packetType) {
        case RHP_TYPE_CTRL_MSG: printf("Control Message"); break;
        case RHP_TYPE_RHMP_MSG: printf("RHMP Message"); break;
        default:                printf("Unknown"); break;
    }
    printf(")\n");


}