// UDPClient.c
//  gcc UDPClient.c -o udpclient
//  ./udpclient
/************* UDP CLIENT CODE *******************/

#include "UDPClient.h"

void sendtoWithFailover(int socketfd, char *msg, size_t msglen, int flags, struct addrinfo *serverInfoList)
{

    for (struct addrinfo *currentServerInfo = serverInfoList; currentServerInfo != NULL; currentServerInfo = currentServerInfo->ai_next)
    {
        int success = sendto(socketfd, msg, msglen, flags, currentServerInfo->ai_addr, currentServerInfo->ai_addrlen);
        if (success >= 0)
            return; // success
    }

    perror("sendto failed, no more addresses to try");
}

int main()
{
    // int clientSocket, nBytes;
    int nBytes;
    char buffer[BUFSIZE];
    struct sockaddr_in clientAddr, serverAddr;

    // /*Create UDP socket*/
    // if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    // {
    //     perror("cannot create socket");
    //     return 0;
    // }

    // /* Bind to an arbitrary return address.
    //  * Because this is the client side, we don't care about the address
    //  * since no application will initiate communication here - it will
    //  * just send responses
    //  * INADDR_ANY is the IP address and 0 is the port (allow OS to select port)
    //  * htonl converts a long integer (e.g. address) to a network representation
    //  * htons converts a short integer (e.g. port) to a network representation */
    // memset((char *)&clientAddr, 0, sizeof(clientAddr));
    // clientAddr.sin_family = AF_INET;
    // clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // clientAddr.sin_port = htons(0);

    struct addrinfo clientHints; // following example in manpage getaddrinfo(3)
    memset(&clientHints, 0, sizeof(clientHints));
    clientHints.ai_family = AF_INET;      // IPv4
    clientHints.ai_socktype = SOCK_DGRAM; // UDP
    clientHints.ai_flags = AI_PASSIVE;    // For wildcard IP address
    clientHints.ai_protocol = 0;          // Any protocol
    clientHints.ai_canonname = NULL;
    clientHints.ai_addr = NULL;
    clientHints.ai_next = NULL;

    struct addrinfo *clientAddrList;
    int success = getaddrinfo(NULL, "0", &clientHints, &clientAddrList); // port 0 means OS chooses free port
    if (success != 0)
    {
        perror("client getaddrinfo failed");
        return 0;
    }

    int clientSocketfd;
    struct addrinfo *currentAddrInfo;
    for (currentAddrInfo = clientAddrList; currentAddrInfo != NULL; currentAddrInfo = currentAddrInfo->ai_next)
    {
        clientSocketfd = socket(currentAddrInfo->ai_family, currentAddrInfo->ai_socktype, currentAddrInfo->ai_protocol);
        if (clientSocketfd == -1)
            continue; // socket creation failed, try next address

        int bindResult = bind(clientSocketfd, currentAddrInfo->ai_addr, currentAddrInfo->ai_addrlen);
        if (bindResult == 0)
            break; // success
        else
        {
            close(clientSocketfd); // current bind failed, close socket and try next address
        }
    }

    freeaddrinfo(clientAddrList); // free the linked list of potential addresses

    if (currentAddrInfo == NULL) // none of the addresses worked
    {
        perror("failed to bind client socket");
        return 0;
    }

    // /* Configure settings in server address struct */
    // memset((char *)&serverAddr, 0, sizeof(serverAddr));
    // serverAddr.sin_family = AF_INET;
    // serverAddr.sin_port = htons(PORT);
    // serverAddr.sin_addr.s_addr = inet_addr(SERVER);
    // memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    struct addrinfo serverHints; // following example in manpage getaddrinfo(3)
    memset(&serverHints, 0, sizeof(serverHints));
    serverHints.ai_family = AF_INET;      // IPv4
    serverHints.ai_socktype = SOCK_DGRAM; // UDP
    serverHints.ai_flags = 0;
    serverHints.ai_protocol = 0; // Any protocol
    serverHints.ai_canonname = NULL;
    serverHints.ai_addr = NULL;
    serverHints.ai_next = NULL;

    struct addrinfo *serverAddrList;
    success = getaddrinfo(SERVERSTR, PORTSTR, &serverHints, &serverAddrList);
    if (success != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(success));
        return 0;
    }
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

//probably want to change this to modify a packet passed in,
// and return a length of the assembled packet,
// this might entail representing a packet as like a 
//byte array instead of a struct, idk 
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
    //uint16_t checksum = computeChecksum16(packet);
    checksum = computeChecksum16(packet)
    return packet;
}

uint computeChecksum16(struct RHP *packet)
{

    uint32_t sum = 0x0000;

    // Add the version and lower half of source port
    sum += ((0x0F & packet->srcPort) << 8) | (0x0F & (uint16_t)packet->version);

    // Add the high half of source port and the lower half of destination port
    sum += ((0xF0 & packet->srcPort)) | (0x0F & (packet->destPort));
}



