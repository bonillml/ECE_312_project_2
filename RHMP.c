#include "RHMP.h"

int createRHMPMessageFromArray(char *msg, uint8_t type, char *mesgOutBuffer, uint16_t lengthOfMsg){
    struct RHMPHeader header;

    if(mesgOutBuffer == NULL || msg == NULL){
        return -1; // Invalid buffer
    }
    
    // Check the type validity
    switch (type)
    {    case RMHMP_MSG_TYPE_RESERVED: // Control message
    case RMHMP_MSG_TYPE_MSG_REQUEST: // RHMP message
    case RMHMP_MSG_TYPE_MSG_RESPONSE: // RHMP message
    case RMHMP_MSG_TYPE_ID_REQUEST: // RHMP message
    case RMHMP_MSG_TYPE_ID_RESPONSE: // RHMP message
        break;
    default:
        return -1; // Invalid type
    }

    header.commID_Type_and_Length = ((COMM_ID & COMM_ID_MASK) << BIT_OFFSET_COMM_ID) |
                                   ((type & 0x3F) << BIT_OFFSET_TYPE) |
                                   ((lengthOfMsg & 0xFFF) << BIT_OFFSET_LENGTH);

    if (lengthOfMsg < RHMP_MIN_PAYLOAD_LENGTH || lengthOfMsg > RHMP_MAX_PAYLOAD_LENGTH)
    {
        return -1; // Invalid length
    }

    memcpy(mesgOutBuffer, &header, RHMP_HEADER_SIZE);
    memcpy(mesgOutBuffer + RHMP_HEADER_SIZE, msg, lengthOfMsg);

    return RHMP_HEADER_SIZE + lengthOfMsg;
}