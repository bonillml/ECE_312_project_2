/************* UDP CLIENT CODE *******************/



#include "UDPClient.h"

int main() {
    int clientSocket, nBytes;
    char buffer[BUFSIZE];
    struct sockaddr_in clientAddr, serverAddr;

    /*Create UDP socket*/
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        return 0;
    }

    /* Bind to an arbitrary return address.
     * Because this is the client side, we don't care about the address 
     * since no application will initiate communication here - it will 
     * just send responses 
     * INADDR_ANY is the IP address and 0 is the port (allow OS to select port) 
     * htonl converts a long integer (e.g. address) to a network representation 
     * htons converts a short integer (e.g. port) to a network representation */
    memset((char *) &clientAddr, 0, sizeof (clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons(0);

    if (bind(clientSocket, (struct sockaddr *) &clientAddr, sizeof (clientAddr)) < 0) {
        perror("bind failed");
        return 0;
    }

    /* Configure settings in server address struct */
    memset((char*) &serverAddr, 0, sizeof (serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    // Prompt the user for a message.
    printf("> Enter the message you'd like to send: \n> ");
    memset(msg_out_buffer, 0, MESSAGE_SIZE);
    fgets(msg_out_buffer, sizeof(msg_out_buffer) - 1, stdin);
    msg_out_buffer[strcspn(msg_out_buffer, "\r\n")] = 0;
    printf("> Sending %s\n", msg_out_buffer);

    // Create and populate the RHP packet struct.
    struct RHP *packet;
    if (!(packet = createRHPPacket(msg_out_buffer, 0))) {
        perror("packet create failed");
        return 0;
    }

    /* send a message to the server */
    if (sendto(clientSocket, msg_out_buffer, strlen(msg_out_buffer), 0, (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
        perror("sendto failed");
        return 0;
    }

    /* Receive message from server */
    nBytes = recvfrom(clientSocket, buffer, BUFSIZE, 0, NULL, NULL);

    printf("> Received from server: %s\n", buffer);

    close(clientSocket);
    return 0;
}

struct RHP* createRHPPacket(char msg[], uint8_t type) {

    struct RHP *packet = malloc(sizeof(struct RHP));

    uint8_t totalOctetCount = DEFAULT_NUM_OCTETS;

    // Assign the RHP version.
    packet->version = RHP_VERSION;

    // Assign the source port.
    packet->srcPort = SOURCE_PORT;

    // Check the type and length validity
    uint16_t length = (sizeof(char) * (strlen(msg) + 1));
    if ((!type || type == 4) && length < 4096) {

        // Valid length, so increment octet count.
        totalOctetCount += length;

        // Populate the length and type octets
        packet->length_and_type = ((0x3 | type) << 3) | (0x0FFF & length);

        // Assign the destination port.
        if (type == 4) packet->destPort = 0x0ECE;
        else packet->destPort = 0x1874;
    }
    else
        return NULL;

    // If there an odd number of octets, add buffer.
    if ((totalOctetCount % 2)) {
        packet->buffer = 1;
        totalOctetCount++;
    }

    // Set total octet count
    packet->totalOctetCount = totalOctetCount;
    printf("> Total packet octet: %d\n", totalOctetCount);
    
    // Compute the packet's checksum 
    computeChecksum(packet);

    return packet;
}

void computeChecksum(struct RHP* packet) {

    uint32_t sum = 0x0000;
    
    // Add the version and lower half of source port
    sum += ((0x0F & packet->srcPort) << 8) | (0x0F & (uint16_t)packet->version);

    // Add the high half of source port and the lower half of destination port
    sum += ((0xF0 & packet->srcPort)) | (0x0F & (packet->destPort));
}
