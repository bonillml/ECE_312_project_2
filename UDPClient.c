// UDPClient.c
//  gcc UDPClient.c SocketHelpers.c -o udpclient
//  ./udpclient
/************* UDP CLIENT CODE *******************/

#include "UDPClient.h"

int main()
{
    // int clientSocket, nBytes;
    int nBytes;
    char buffer[BUFSIZE];
    //struct sockaddr_in clientAddr, serverAddr;

    //reserve a UDP port for the client, and get its socket file descriptor
    int clientSocketfd = reserveClientPort(AF_INET, SOCK_DGRAM, AI_PASSIVE, IPPROTO_UDP);
    if (clientSocketfd == 0)
    {
        perror("client port reservation failed");
        return 0;
    }

    // /* get linked list of possible addresses for server */
    struct addrinfo *serverAddrList = resolveServerAddr(AF_INET, SOCK_DGRAM, 0, IPPROTO_UDP, (char *)SERVERSTR, (char *)PORTSTR);

    // Prompt the user for a message.
    printf("> Enter the message you'd like to send: \n> ");
    memset(msg_out_buffer, 0, MESSAGE_SIZE);
    fgets(msg_out_buffer, sizeof(msg_out_buffer) - 1, stdin);
    msg_out_buffer[strcspn(msg_out_buffer, "\r\n")] = 0;
    printf("> Sending %s\n", msg_out_buffer);

    // Create and populate the RHP packet struct.
    struct RHP *packet;
    if (!(packet = createRHPPacket(msg_out_buffer, 0)))
    {
        perror("packet create failed");
        return 0;
    }

    /* send a message to the server */
    sendtoWithFailover(clientSocketfd, msg_out_buffer, strlen(msg_out_buffer), 0, serverAddrList);

    /* Receive message from server */
    nBytes = recvfrom(clientSocketfd, buffer, BUFSIZE, 0, NULL, NULL);

    printf("> Received from server: %s\n", buffer);

    close(clientSocketfd);
    return 0;
}

// probably want to change this to modify a packet passed in,
//  and return a length of the assembled packet,
//  this might entail representing a packet as like a
// byte array instead of a struct, idk
struct RHP *createRHPPacket(char msg[], uint8_t type)
{

    struct RHP *packet = malloc(sizeof(struct RHP));

    uint8_t totalOctetCount = DEFAULT_NUM_OCTETS;

    // Assign the RHP version.
    packet->version = RHP_VERSION;

    // Assign the source port.
    packet->srcPort = SOURCE_PORT;

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
    computeChecksum16(packet);
    return packet;
}

void computeChecksum16(struct RHP *packet)
{

    uint32_t sum = 0x0000;

    // Add the version and lower half of source port
    sum += ((0x0F & packet->srcPort) << 8) | (0x0F & (uint16_t)packet->version);

    // Add the high half of source port and the lower half of destination port
    sum += ((0xF0 & packet->srcPort)) | (0x0F & (packet->destPort));
}
