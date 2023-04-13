//
// Created by 史导的Mac on 2023/4/5.
//

#ifndef MQTT_PROTOCOL_TEST_MQTTHELPER_H
#define MQTT_PROTOCOL_TEST_MQTTHELPER_H

/**
 * The data for a length delimited string
 */
typedef struct
{
    int len; /**< the length of the string */
    char* data; /**< pointer to the string data */
} MQTTLenString;

int readInt(char** pptr);
unsigned char readChar(char** pptr);
void writeChar(char** pptr, char c);
void writeInt(char** pptr, int anInt);
void writeData(char** pptr, const void* data, int datalen);
void writeInt4(char **pptr, int anInt);
int readInt4(char **pptr);
void writeMQTTLenString(char** pptr, MQTTLenString lenstring);
int MQTTLenStringRead(MQTTLenString* lenstring, char** pptr, const char* enddata);
int MQTTPacket_encode(char *buf, size_t length);
int MQTTPacket_VBIlen(int rem_len);
int MQTTPacket_decodeBuf(char* buf, unsigned int* value);

#endif //MQTT_PROTOCOL_TEST_MQTTHELPER_H
