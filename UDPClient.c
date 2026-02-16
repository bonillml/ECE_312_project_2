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

    while (1)
    {

        // Prompt the user for RHP or RHMP
        int option = 0;
        while (!option || option > 4)
        {
            printf("\n> (1) RHP, (2) RHMP (Msg. Req.), (3) RHMP (ID Req.),or (4) EXIT: ");
            memset(msg_out_buffer, 0, sizeof(msg_out_buffer));
            fgets(msg_out_buffer, 8, stdin); // Read in at most one byte of data (should be enough for an int.
            option = atoi(msg_out_buffer);
        }
        if (option == 4)
            break;

        int lengthOfMsgOut;
        int RHPtype;
        if (option == 1)
        {

            // Prompt the user for a message.
            printf("> Enter the message you'd like to send: ");
            memset(msg_out_buffer, 0, sizeof(msg_out_buffer));
            fgets(msg_out_buffer, RHP_MAX_PAYLOAD_LENGTH - 1, stdin); // read up to 4094 characters from stdin (leave space for padding and null terminator)
            lengthOfMsgOut = strcspn(msg_out_buffer, "\r\n");
            msg_out_buffer[lengthOfMsgOut] = 0; // null terminate the string
            lengthOfMsgOut += 1;                // account for null terminator in length
            RHPtype = RHP_TYPE_CTRL_MSG;
            printf("> Sending: %s\n", msg_out_buffer);
        }
        else if (option == 2)
        {

            // Construct RHMP message request.
            memset(msg_out_buffer, 0, sizeof(msg_out_buffer));
            struct RHMPFields msgFields = {
                .commID = COMM_ID,
                .type = RMHMP_MSG_TYPE_MSG_REQUEST,
                .length = 0};
            lengthOfMsgOut = writeRHMPmsgToBuffer(&msgFields, msg_out_buffer, sizeof(msg_out_buffer));
            RHPtype = RHP_TYPE_RHMP_MSG;
            printf("> Sending RHMP message request.\n");
        }
        else if (option == 3)
        {

            // Construct RHMP ID request.
            memset(msg_out_buffer, 0, sizeof(msg_out_buffer));
            struct RHMPFields msgFields = {
                .commID = COMM_ID,
                .type = RMHMP_MSG_TYPE_ID_REQUEST,
                .length = 0};
            lengthOfMsgOut = writeRHMPmsgToBuffer(&msgFields, msg_out_buffer, sizeof(msg_out_buffer));
            RHPtype = RHP_TYPE_RHMP_MSG;
            printf("> Sending RHMP ID request.\n");
        }

        // Create and populate the RHP packet struct.
        char packetOutBuffer[RHP_MAX_MESSAGE_SIZE];
        memset(packetOutBuffer, 0, sizeof(packetOutBuffer));
        int sizeToSend = createRHPPacketFromArray(msg_out_buffer, RHPtype, packetOutBuffer, lengthOfMsgOut);
        if (sizeToSend < 0)
        {
            perror("packet assembly failed");
            return 0;
        }
        // Print the packet info for debugging.
        // if(printRHPPacketInfo(packetOutBuffer, sizeToSend) < 0){
        //     close(clientSocketfd);
        //     return -1;
        // }

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

        if (printRHPPacketInfo(packetInBuffer, nBytesReceived) < 0)
        {
            close(clientSocketfd);
            return -1;
        }

        struct RHPFields receivedPacketFields;
        if (parseRHPInfoFromBuffer(packetInBuffer, nBytesReceived, &receivedPacketFields) < 0)
        {
            close(clientSocketfd);
            return -1;
        }
        if (receivedPacketFields.type == RHP_TYPE_RHMP_MSG)
        {
            printf("Interpreting payload as RHMP message...\n");
            printRHMPPacket(packetInBuffer + receivedPacketFields.payloadOffset, receivedPacketFields.length);
        }
        else
        {
            bool isNullTerminated = isPacketPayloadNullTerminated(packetInBuffer, nBytesReceived);
            if (isNullTerminated < 0)
            {
                close(clientSocketfd);
                return -1;
            }

            if (printRHPPacketPayload(packetInBuffer, isNullTerminated, nBytesReceived) < 0)
            {
                close(clientSocketfd);
                return -1;
            }
        }

    } // End while(1) loop

    close(clientSocketfd);

    return 0;
}
