#pragma once
// SocketHelpers.h

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int reserveClientPort(int protocolFamily, int socketType, int portFlags, int protocol);
struct addrinfo *resolveServerAddr(int protocolFamily, int socketType, int portFlags, int protocol, char *serverStr, char *portStr);
void sendtoWithFailover(int socketfd, char *msg, size_t msglen, int flags, struct addrinfo *serverInfoList);
