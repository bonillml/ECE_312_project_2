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
    char msg_out_buffer[RHP_OFFSET_SIZE + RHP_MAX_PAYLOAD_LENGTH];
    char msg_in_buffer[RHP_MESSAGE_SIZE];
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
    printf("> Enter the message you'd like to send: \n> ");
    memset(msg_out_buffer, 0, sizeof(msg_out_buffer));
    fgets(msg_out_buffer, RHP_MAX_PAYLOAD_LENGTH - 1, stdin); // read up to 4094 characters from stdin (leave space for padding and null terminator)
    int lengthOfMsgOut = strcspn(msg_out_buffer, "\r\n");
    msg_out_buffer[lengthOfMsgOut] = 0; // null terminate the string
    lengthOfMsgOut += 1;                // account for null terminator in length
    printf("> Sending %s\n", msg_out_buffer);

    // Create and populate the RHP packet struct.
    struct RHPHeader header;
    char packetOutBuffer[RHP_MESSAGE_SIZE];
    memset(packetOutBuffer, 0, sizeof(packetOutBuffer));
    int sizeToSend = createRHPPacketFromArray(msg_out_buffer, 0, packetOutBuffer, lengthOfMsgOut, &header);
    if (sizeToSend < 0)
    {
        perror("packet assembly failed");
        return 0;
    }
    printRHPPacket(packetOutBuffer, false);

    struct pollfd fds;
    fds.fd = clientSocketfd;
    fds.events = POLLIN;
    for (int numTriesSinceResponse = 0; numTriesSinceResponse < NUM_RETRIES; numTriesSinceResponse++)
    {
        /* send a message to the server */
        // sendtoWithFailover(clientSocketfd, msg_out_buffer, strlen(msg_out_buffer), 0, serverAddrList);
        sendtoWithFailover(clientSocketfd, packetOutBuffer, sizeToSend, 0, serverAddrList);

        int pollResult = poll(&fds, 1, TIMEOUT_MS); // wait up to 100ms for data
        if (pollResult == 0)
        {
            printf("> No response from server within timeout period. (Attempts since last response: %d)\n", numTriesSinceResponse + 1);
            continue;
        }
        else if (pollResult < 0)
        {
            perror("poll error");
            close(clientSocketfd);
            return 0;
        }
        else
        {
            // Data is available to read
            /* Receive message from server */
            nBytes = recvfrom(clientSocketfd, buffer, BUFSIZE, 0, NULL, NULL);
            //probably should just have a struct for the header instead of a normal buffer, and then blocking recvfrom until we get the full header + payload + checksum
            
            if (nBytes < 0)
            {
                perror("Error receiving data from server");
                close(clientSocketfd);
                return 0;
            }
            printf("> Received from server: %s\n", buffer);
            break;
        }
    }
    close(clientSocketfd);

    return 0;
}
