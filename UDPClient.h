#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

// DON'T TOUCH THESE SETTINGS
#define SERVER "137.112.38.47"
#define PORT 2526
#define BUFSIZE 1024

// RHP Packet Settings (touchable)
#define DEFAULT_NUM_OCTETS 9
#define RHP_VERSION 12
#define SOURCE_PORT 3285
#define CTRL_MSG_PORT 0x1874
#define RHMP_MSG_PORT 0x0ECE

// Message settings (touchable)
#define MESSAGE_SIZE 256
char msg_out_buffer[MESSAGE_SIZE];
char msg_in_buffer[MESSAGE_SIZE];