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


int printRHMPPacket(char *bufferIn, uint16_t lengthOfBufferIn)
{
    struct RHMPFields fields;
    int result = readRHMPMessageFromBuffer(bufferIn, &fields, bufferIn, lengthOfBufferIn);
    if (result < 0)
    {
        return -1; // Failed to read message from buffer
    }

        if (fields.length < RHMP_MIN_PAYLOAD_LENGTH || fields.length > RHMP_MAX_PAYLOAD_LENGTH)
    {
        return -1; // Invalid length
    }

    printf("RHMP Packet:\n");
    printf("  CommID: %u (0x%04X)\n", fields.commID, fields.commID);
    printf("  Type: %u (0x%02X)\n", fields.type, fields.type);
    printf("  Length: %u (0x%04X)\n", fields.length, fields.length);
    switch (fields.type)
    {    case RMHMP_MSG_TYPE_RESERVED:
        printf("  Type Description: Reserved\n");
        printf("  This type is reserved for control messages and should not contain any payload.\n");
        break;
    case RMHMP_MSG_TYPE_MSG_REQUEST:
        printf("  Type Description: RHMP message request\n"); 
        printf("  This type is used for sending a message request and should not contain any payload.\n");
        break;
    case RMHMP_MSG_TYPE_MSG_RESPONSE:
        printf("  Type Description: RHMP message response\n");
        //this type responds with an ascii string in the payload, so we can print that out
        if (fields.length == 0) {
            printf("  No payload for message response.\n");
        } else {
            char payload[RHMP_MAX_PAYLOAD_LENGTH + 1]; // +1 for null terminator
            memcpy(payload, bufferIn + RHMP_HEADER_SIZE, fields.length);
            payload[fields.length] = '\0'; // Null-terminate the string
            printf("  Payload: %s\n", payload);
        }
        break;
    case RMHMP_MSG_TYPE_ID_REQUEST:
        printf("  Type Description: RHMP ID request\n");
        printf("  This type is used for requesting an ID and should not contain any payload.\n");
        break;
    case RMHMP_MSG_TYPE_ID_RESPONSE:
        printf("  Type Description: RHMP ID response\n");
        //this type responds with a 32-bit ID in the payload, so we can print that out
        if (fields.length != 4) {
            printf("  Invalid payload length for ID response. Expected 4 bytes for the ID.\n");
        } else {
            uint32_t id;
            memcpy(&id, bufferIn + RHMP_HEADER_SIZE, sizeof(uint32_t));
            printf("  ID in Payload: %u (0x%08X)\n", id, id);        }
        break;
    default:
        printf("  Type Description: Unknown\n");
        break;
    }




    return 0;
}