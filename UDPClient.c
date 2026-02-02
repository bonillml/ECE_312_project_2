/************* UDP CLIENT CODE *******************/

#include "UDPClient.h"

struct RHP {
    uint8_t version;    // Version number of RHP protocol.
    uint16_t srcPort;   // RHP port of source.
    uint16_t destPort;  // Destination RHP port.

    // Keep length values in range 0 - 4095
    // Keep type values in range 0 - 15
    // 0 = Control message (ASCII Strings)
    //      (Use 0x1874 for the dstPort)
    // 4 = RHMP message
    //      (Use 0xECE for the dstPort)
    // The dstPort will be automatically assigned given the type.
    //
    // The length and type are combined to form two octets.
    uint16_t length_and_type;    // Length of payload (bytes) & RHP message type (payload protocol)

    // If zero, this RHP packet won't have a buffer. A non-zero
    // value means there is a buffer.
    uint8_t buffer;     // Optional 8-bit buffer of zeros to ensure even number of octects in packet.

    // Has a variable size.
    // May need to change it from a uint16_t
    uint64_t payload;   // RHP SDU (depedent on type)

    uint16_t checksum;  // 16-bit internet checksum.

};

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
    msg_out_buffer[sizeof(msg_out_buffer) - 1] = 0;
    printf("> Sending %s\n", msg_out_buffer);

    // Create and populate the RHP packet struct.
    struct RHP *packet;
    if (!(packet = createRHPPacket(msg_out_buffer, 0))) {
        perror("packet create failed");
        return 0;
    }

    /* send a message to the server */
    if (sendto(clientSocket, msg_out_buffer, strlen(msg_out_buffer), 0,
            (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
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

    struct RHP *packet;

    // Assign the RHP version.
    packet->version = RHP_VERSION;

    // Assign the source port.
    packet->srcPort = SOURCE_PORT;

    // Check the type and length validity
    uint16_t length = sizeof(msg) / sizeof(char);
    if ((!type || type == 4) && length < 4096) {

        // Populate the length and type octets
        packet->length_and_type = ((0x3 | type) << 3) | (0x0FFF & length);

        // Assign the destination port.
        if (type == 4) packet->destPort = 0x0ECE;
        else packet->destPort = 0x1874;
    }
    else
        return 0;

    

    return packet;
}
