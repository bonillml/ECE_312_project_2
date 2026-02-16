#include "RHMP.h"

int writeRHMPmsgToBuffer(struct RHMPFields *msg, char *msgOutBuffer, uint16_t bufferLength)
{
    struct RHMPHeaderRaw header;

    if (msgOutBuffer == NULL || msg == NULL)
    {
        return -1; // Invalid buffer
    }
    if (bufferLength < RHMP_HEADER_SIZE + msg->length)
    {
        return -1; // Buffer too small
    }
    if (msg->length < RHMP_MIN_PAYLOAD_LENGTH || msg->length > RHMP_MAX_PAYLOAD_LENGTH)
    {
        return -1; // Invalid length
    }

    // Check the type validity
    switch (msg->type)
    {
    case RMHMP_MSG_TYPE_RESERVED:    // Control message
        if(msg->length != 0){
            return -1; // Invalid length for message request
        }
        break;
    case RMHMP_MSG_TYPE_MSG_REQUEST: // RHMP message
        if(msg->length != 0){
            return -1; // Invalid length for message request
        }
        break;
    case RMHMP_MSG_TYPE_MSG_RESPONSE: // RHMP message
        break;
    case RMHMP_MSG_TYPE_ID_REQUEST:   // RHMP message
            if(msg->length != 0){
            return -1; // Invalid length for message request
        }
        break;
    case RMHMP_MSG_TYPE_ID_RESPONSE:  // RHMP message

        break;
    default:
        return -1; // Invalid type
    }

    header.commID_Type_and_Length = ((msg->commID & COMM_ID_MASK) << BIT_OFFSET_COMM_ID) |
                                    ((msg->type & 0x3F) << BIT_OFFSET_TYPE) |
                                    ((msg->length & 0xFFF) << BIT_OFFSET_LENGTH);
    if (msg->length < RHMP_MIN_PAYLOAD_LENGTH || msg->length > RHMP_MAX_PAYLOAD_LENGTH)
    {
        return -1; // Invalid length
    }

    memcpy(msgOutBuffer, &header, RHMP_HEADER_SIZE);
    memcpy(msgOutBuffer + RHMP_HEADER_SIZE, msg, msg->length);

    return RHMP_HEADER_SIZE + msg->length;
}

int readRHMPMessageFromBuffer(char *msg, struct RHMPFields *fields, char *msgInBuffer, uint16_t lengthOfMsgInBuffer)
{
    struct RHMPHeaderRaw header;
    if (msgInBuffer == NULL || fields == NULL)
    {
        return -1; // Invalid buffer or fields pointer
    }

    memcpy(&header, msgInBuffer, RHMP_HEADER_SIZE);

    fields->commID = (header.commID_Type_and_Length >> BIT_OFFSET_COMM_ID) & COMM_ID_MASK;
    fields->type = (header.commID_Type_and_Length >> BIT_OFFSET_TYPE) & 0x3F;
    fields->length = (header.commID_Type_and_Length >> BIT_OFFSET_LENGTH) & 0xFFF;

    if (fields->length < RHMP_MIN_PAYLOAD_LENGTH || fields->length > RHMP_MAX_PAYLOAD_LENGTH)
    {
        return -1; // Invalid length
    }
    memcpy(msg, msgInBuffer + RHMP_HEADER_SIZE, fields->length);

    return fields->length;
}