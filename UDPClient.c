// UDPClient.c
//  gcc UDPClient.c SocketHelpers.c RHPPacket.c -o udpclient
//  ./udpclient
/************* UDP CLIENT CODE *******************/

#include "UDPClient.h"

int main()
{
    // int clientSocket, nBytes;
    int nBytes;
    char buffer[BUFSIZE];
    char msg_out_buffer[RHP_OFFSET_SIZE + RHP_MAX_PAYLOAD_LENGTH];
    char msg_in_buffer[RHP_MAX_MESSAGE_SIZE];
    // struct sockaddr_in clientAddr, serverAddr;

    // reserve a UDP port for the client, and get its socket file descriptor
    int clientSocketfd = reserveClientPort(AF_INET, SOCK_DGRAM, AI_PASSIVE, IPPROTO_UDP);
    if (clientSocketfd == 0)
    {
        perror("client port reservation failed");
        return 0;
    }

    // /* get linked list of possible addresses for server */
    struct addrinfo *serverAddrList = resolveServerAddr(AF_INET, SOCK_DGRAM, 0, IPPROTO_UDP, (char *)SERVERSTR, (char *)PORTSTR);

    // Prompt the user for a message.
    printf("> Enter the message you'd like to send: ");
    memset(msg_out_buffer, 0, sizeof(msg_out_buffer));
    fgets(msg_out_buffer, RHP_MAX_PAYLOAD_LENGTH - 1, stdin); // read up to 4094 characters from stdin (leave space for padding and null terminator)
    int lengthOfMsgOut = strcspn(msg_out_buffer, "\r\n");
    msg_out_buffer[lengthOfMsgOut] = 0; // null terminate the string
    lengthOfMsgOut += 1;                // account for null terminator in length
    printf("> Sending %s\n", msg_out_buffer);

    // Create and populate the RHP packet struct.
    char packetOutBuffer[RHP_MAX_MESSAGE_SIZE];
    memset(packetOutBuffer, 0, sizeof(packetOutBuffer));
    int sizeToSend = createRHPPacketFromArray(msg_out_buffer, 0, packetOutBuffer, lengthOfMsgOut);
    if (sizeToSend < 0)
    {
        perror("packet assembly failed");
        return 0;
    }
    if(printRHPPacketInfo(packetOutBuffer, sizeToSend) < 0){
        close(clientSocketfd);
        return -1;
    }

    char packetInBuffer[RHP_MAX_MESSAGE_SIZE];
    struct RHPHeader *receivedPacketHeader = (struct RHPHeader *)packetInBuffer;
    memset(packetInBuffer, 0, sizeof(packetInBuffer));
    int nBytesReceived = sendPacketGetAck(clientSocketfd, serverAddrList, packetOutBuffer, sizeToSend, packetInBuffer, sizeof(packetInBuffer), SEND_TIMEOUT_MS, SEND_NUM_RETRIES);
    if (nBytesReceived <= 0)
    {
        fprintf(stderr, "Error receiving ACK from server\n");
        close(clientSocketfd);
        return -1;
    }
    printf("\nReceived Response from Server:\n");

    if(printRHPPacketInfo(packetInBuffer, nBytesReceived) < 0){
        close(clientSocketfd);
        return -1;
    }
    
    bool isNullTerminated = isPacketPayloadNullTerminated(packetInBuffer, nBytesReceived);
    if(isNullTerminated < 0){
        close(clientSocketfd);
        return -1;
    }
    
    if(printRHPPacketPayload(packetInBuffer, isNullTerminated, nBytesReceived) < 0){
        close(clientSocketfd);
        return -1;
    }

    close(clientSocketfd);

    return 0;
}
