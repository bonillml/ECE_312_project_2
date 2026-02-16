#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define BIT_OFFSET_COMM_ID 0
#define COMM_ID_MASK 0x00003FFF
#define BIT_OFFSET_TYPE 14
#define TYPE_MASK 0x000FC000
#define BIT_OFFSET_LENGTH 20
#define LENGTH_MASK 0xFFF00000
#define BIT_OFFSET_LENGTH 20

typedef struct RHMPHeader { uint32_t commID_Type_and_Length;};