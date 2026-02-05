#include "SocketHelpers.h"


// returns socket file descriptor bound to a free port
int reserveClientPort(int protocolFamily, int socketType, int portFlags, int protocol)
{
    struct addrinfo clientHints; // following example in manpage getaddrinfo(3)
    memset(&clientHints, 0, sizeof(clientHints));
    clientHints.ai_family = protocolFamily;
    clientHints.ai_socktype = socketType;
    clientHints.ai_flags = portFlags;
    clientHints.ai_protocol = protocol;
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

    return clientSocketfd;
}
//
struct addrinfo *resolveServerAddr(int protocolFamily, int socketType, int portFlags, int protocol, char *serverStr, char *portStr)
{
    struct addrinfo serverHints; // following example in manpage getaddrinfo(3)
    memset(&serverHints, 0, sizeof(serverHints));
    serverHints.ai_family = protocolFamily;
    serverHints.ai_socktype = socketType;
    serverHints.ai_flags = portFlags;
    serverHints.ai_protocol = protocol;
    serverHints.ai_canonname = NULL;
    serverHints.ai_addr = NULL;
    serverHints.ai_next = NULL;

    struct addrinfo *serverAddrList;
    int success = getaddrinfo(serverStr, portStr, &serverHints, &serverAddrList);
    if (success != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(success));
        return 0;
    }
    return serverAddrList;
}

// tries to send msg to each address in serverInfoList until one succeeds
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
