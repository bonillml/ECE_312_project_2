#include "RHPPacket.h"

// Assembles an RHP packet into the provided buffer, including checksum
int createRHPPacketFromArray(char *msg, uint8_t type, char *packetOutBuffer, uint16_t lengthOfMsg)
{
    struct RHPHeader header;   
    header.version = RHP_VERSION;
    printf("> RHP Version: %d\n", header.version);
    header.srcPort = RHP_SOURCE_PORT;

    type &= 0x0F; // Ensure type is 4 bits
    switch (type)
    {
    case RHP_TYPE_CTRL_MSG:
        header.destPort = RHP_PORT_CTRL_MSG;
        break;
    case RHP_TYPE_RHMP_MSG:
        header.destPort = RHP_PORT_RHMP_MSG;
        break;
    default:
        return -1; // Invalid type
    }

    lengthOfMsg &= 0x0FFF; // Convert to bytes

    if (lengthOfMsg < 0 || lengthOfMsg > RHP_MAX_PAYLOAD_LENGTH)
    {
        return -1; // Invalid length
    }

    header.length_and_type = (0x0FFF & lengthOfMsg) | ((type & 0x0F) << 12); ;

    size_t oddOffset = (lengthOfMsg + 1) % 2; // offset is 1 if payload is even, 0 if odd, to ensure even total octet count
    memcpy(packetOutBuffer, &header, sizeof(struct RHPHeader));
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
        uint16_t word = msg[i] & 0xFF; // high byte
        if (i + 1 < length)
        {
            word |= (uint16_t)(msg[i + 1] & 0xFF) << 8; // low byte
        }
        sum += word;
    }

    if (sum >> 16) //sum exceeds 16 bits, so wrap around carry
    {
        sum = (sum & 0x0000FFFF) + (sum >> sizeof(uint16_t) * 8); // handle carry
    }
    return (uint16_t)(~sum & 0xFFFF); // One's complement
}


void checkSumTester(void){
    uint16_t testMsg[] = {0x4500, 0x0073, 0x0000, 0x4000, 0x4011, 0xc0a8, 0x0001, 0xc0a8, 0x00c7};
    uint16_t correctChecksum = 0xb861;
    uint16_t calculatedChecksum = calculateChecksum((char *)testMsg, sizeof(testMsg));
    printf("> Calculated checksum: 0x%04X\n", calculatedChecksum);
    if (calculatedChecksum == correctChecksum)
        printf("> Checksum test passed!\n");
    else
        printf("> Checksum test failed!\n");
}

void printRHPPacketInfo(const char *packetBuffer)
{
    const uint8_t *rawPacket = (const uint8_t *)packetBuffer;
    struct RHPHeader *packetHeader = (struct RHPHeader *)rawPacket;

    uint16_t payloadLengthValue = (packetHeader->length_and_type) & 0x0FFF;;
    uint8_t packetType = (packetHeader->length_and_type >> 12) & 0x0F;

    printf("RHP Packet:\n");
    printf("Byte order: Little Endian\n");
    
    printf(" Version: %d\n", packetHeader->version);
    printf("  Value : %u\n", packetHeader->version);
    printf("  Raw   : %02X\n", rawPacket[0]);

    printf(" Source Port: %d\n", packetHeader->srcPort);
    printf("  Value : %u\n", packetHeader->srcPort);
    printf("  Raw   : %02X %02X\n", rawPacket[1], rawPacket[2]);

    printf(" Destination Port: %d\n", packetHeader->destPort);
    printf("  Value : %u\n", packetHeader->destPort);
    printf("  Raw   : %02X %02X\n", rawPacket[3], rawPacket[4]);

    printf(" Length and Type: %d\n", packetHeader->length_and_type);
    printf("  Value (combined): %u\n", packetHeader->length_and_type);
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

//returns -1 on error
int sendPacketGetAck(int socketfd, struct addrinfo *serverAddr, char *packetOutBuffer, size_t packetSize, char *packetInBuffer, size_t maxPacketInSize, int timeoutMs, int maxRetries)
{
    int nBytesReceived = 0;
    int lengthOfPayload = 0;
    struct RHPHeader *recvHeader = (struct RHPHeader *)packetInBuffer; // the first part of the incoming packet is the header

    struct pollfd fds;
    fds.fd = socketfd;
    fds.events = POLLIN;

    for (int numTriesSinceResponse = 0; numTriesSinceResponse < maxRetries; numTriesSinceResponse++)
    {
        /* send a message to the server */
        sendtoWithFailover(socketfd, packetOutBuffer, packetSize, 0, serverAddr);
        int pollResult = poll(&fds, 1, timeoutMs);\
        if (pollResult == 0)
        {
            printf("> No response from server within timeout period. (Attempts since last response: %d)\n", numTriesSinceResponse + 1);
            continue;
        }
        else if (pollResult < 0)
        {
            perror("poll error");
            return -1;
        }
        else
        {
            // Data is available to read
            /* Receive message from server */
            //read just recvfrom to first read header, then read rest based on length
            nBytesReceived = recvfrom(socketfd, packetInBuffer, maxPacketInSize, 0, NULL, NULL);
            //probably should just have a struct for the header instead of a normal buffer, and then blocking recvfrom until we get the full header + payload + checksum

            if (nBytesReceived < 0)
            {
                perror("Error receiving data from server");
                return -1;
            }
            lengthOfPayload = (recvHeader->length_and_type) & 0x0FFF;
            size_t evenOffset = (lengthOfPayload & 0x01) ? 0 : 1; // offset is 1 if payload is even, 0 if odd, to ensure even total octet count
            size_t totalExpectedSize = sizeof(struct RHPHeader) + evenOffset + lengthOfPayload + sizeof(uint16_t); // header + optional buffer + payload + checksum

            printf("> Received %d bytes from server.\n", nBytesReceived);

            if(totalExpectedSize > maxPacketInSize)
            {
                fprintf(stderr, "Received packet size exceeds buffer size: %zu recv vs %zu max\n", totalExpectedSize, maxPacketInSize);
                return -1;
            }

            if(nBytesReceived < sizeof(struct RHPHeader))
            {
                fprintf(stderr, "Received packet size smaller than RHP header size: %d actual vs %zu min\n", nBytesReceived, sizeof(struct RHPHeader));
                return -1;
            }

            if(nBytesReceived < RHP_MIN_MESSAGE_SIZE)
             {
                fprintf(stderr, "Received complete header, but packet size smaller than RHP minimum packet size: %d actual vs %zu min\n", nBytesReceived, RHP_MIN_MESSAGE_SIZE);
                printRHPPacketInfo(packetInBuffer);
                return -1;
             }

            if(nBytesReceived < totalExpectedSize)
            {
                fprintf(stderr, "Received packet size does not match expected size: %d recv vs %zu expected\n", nBytesReceived, totalExpectedSize);
                return -1;
            }

            printf("> Expected total packet size: %zu bytes\n", totalExpectedSize);
            uint16_t calculatedChecksum = calculateChecksum(packetInBuffer, totalExpectedSize - sizeof(uint16_t));
            uint16_t receivedChecksum = *((uint16_t *)(packetInBuffer + totalExpectedSize - sizeof(uint16_t)));
            if(calculatedChecksum != receivedChecksum)
            {
                fprintf(stderr, "Received packet has invalid checksum\n");
                fprintf(stderr, "Expected checksum: 0x%04X\n", calculatedChecksum);
                fprintf(stderr, "Received checksum: 0x%04X\n", receivedChecksum);
                return -1;
            }

            break;
        }
    }
    return nBytesReceived;
}