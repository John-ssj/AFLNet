//
// Created by 史导的Mac on 2023/4/5.
//

#include <string.h>

#include "MQTTHelper.h"

/**
 * Calculates an integer from two bytes read from the input buffer
 * @param pptr pointer to the input buffer - incremented by the number of bytes used & returned
 * @return the integer value calculated
 */
int readInt(char **pptr) {
    char *ptr = *pptr;
    int len = 256 * ((unsigned char) (*ptr)) + (unsigned char) (*(ptr + 1));
    *pptr += 2;
    return len;
}


/**
 * Reads one character from the input buffer.
 * @param pptr pointer to the input buffer - incremented by the number of bytes used & returned
 * @return the character read
 */
unsigned char readChar(char **pptr) {
    unsigned char c = **pptr;
    (*pptr)++;
    return c;
}


/**
 * Writes one character to an output buffer.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param c the character to write
 */
void writeChar(char **pptr, char c) {
    **pptr = c;
    (*pptr)++;
}


/**
 * Writes an integer as 2 bytes to an output buffer.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param anInt the integer to write
 */
void writeInt(char **pptr, int anInt) {
    **pptr = (char) (anInt / 256);
    (*pptr)++;
    **pptr = (char) (anInt % 256);
    (*pptr)++;
}


/**
 * Writes length delimited data to an output buffer
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param data the data to write
 * @param datalen the length of the data to write
 */
void writeData(char **pptr, const void *data, int datalen) {
    writeInt(pptr, datalen);
    memcpy(*pptr, data, datalen);
    *pptr += datalen;
}


/**
 * Writes an integer as 4 bytes to an output buffer.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param anInt the integer to write
 */
void writeInt4(char **pptr, int anInt) {
    **pptr = (char) (anInt / 16777216);
    (*pptr)++;
    anInt %= 16777216;
    **pptr = (char) (anInt / 65536);
    (*pptr)++;
    anInt %= 65536;
    **pptr = (char) (anInt / 256);
    (*pptr)++;
    **pptr = (char) (anInt % 256);
    (*pptr)++;
}


/**
 * Calculates an integer from two bytes read from the input buffer
 * @param pptr pointer to the input buffer - incremented by the number of bytes used & returned
 * @return the integer value calculated
 */
int readInt4(char **pptr) {
    unsigned char *ptr = (unsigned char *) *pptr;
    int value = 16777216 * (*ptr) + 65536 * (*(ptr + 1)) + 256 * (*(ptr + 2)) + (*(ptr + 3));
    *pptr += 4;
    return value;
}


void writeMQTTLenString(char **pptr, MQTTLenString lenstring) {
    writeInt(pptr, lenstring.len);
    memcpy(*pptr, lenstring.data, lenstring.len);
    *pptr += lenstring.len;
}


int MQTTLenStringRead(MQTTLenString *lenstring, char **pptr, const char *enddata) {
    int len = -1;

    /* the first two bytes are the length of the string */
    if (enddata - (*pptr) > 1) /* enough length to read the integer? */
    {
        lenstring->len = readInt(pptr); /* increments pptr to point past length */
        if (&(*pptr)[lenstring->len] <= enddata) {
            lenstring->data = (char *) *pptr;
            *pptr += lenstring->len;
            len = 2 + lenstring->len;
        }
    }
    return len;
}


/**
 * Encodes the message length according to the MQTT algorithm
 * @param buf the buffer into which the encoded data is written
 * @param length the length to be encoded
 * @return the number of bytes written to buffer
 */
int MQTTPacket_encode(char *buf, size_t length) {
    int rc = 0;

    do {
        char d = length % 128;
        length /= 128;
        /* if there are more digits to encode, set the top bit of this digit */
        if (length > 0)
            d |= 0x80;
        if (buf)
            buf[rc++] = d;
        else
            rc++;
    } while (length > 0);
    return rc;
}

int MQTTPacket_VBIlen(int rem_len) {
    int rc = 0;

    if (rem_len < 128)
        rc = 1;
    else if (rem_len < 16384)
        rc = 2;
    else if (rem_len < 2097152)
        rc = 3;
    else
        rc = 4;
    return rc;
}

/**
 * Decodes the message length according to the MQTT algorithm
 * @param getcharfn pointer to function to read the next character from the data source
 * @param value the decoded length returned
 * @return the number of bytes read from the socket
 */
int MQTTPacket_VBIdecode(int (*getcharfn)(char *, int), unsigned int *value) {
    char c;
    int multiplier = 1;
    int len = 0;
#define MAX_NO_OF_REMAINING_LENGTH_BYTES 4

    *value = 0;
    do {
        int rc = 0;

        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES) {
            goto exit;
        }
        rc = (*getcharfn)(&c, 1);
        if (rc != 1)
            goto exit;
        *value += (c & 127) * multiplier;
        multiplier *= 128;
    } while ((c & 128) != 0);
    exit:
    return len;
}


int MQTTPacket_decodeBuf(char *buf, unsigned int *value) {
    char *buf_copy = buf;
    char c;
    int multiplier = 1;
    int len = 0;
#define MAX_NO_OF_REMAINING_LENGTH_BYTES 4

    *value = 0;
    do {
        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES) {
            goto exit;
        }
        c = *buf_copy++;
        *value += (c & 127) * multiplier;
        multiplier *= 128;
    } while ((c & 128) != 0);
    exit:
    return len;
}

