//
// Created by 史导的Mac on 2023/4/3.
//

#ifndef MQTT_PROTOCOL_TEST_MQTTPARSER_H
#define MQTT_PROTOCOL_TEST_MQTTPARSER_H

#include "stdint.h"
#include "stdbool.h"
#include "MQTTProperties.h"

typedef struct {
    uint8_t fixed_header;
    uint32_t remaining_length;
    uint16_t protocol_name_len;
    char protocol_name[4];
    uint8_t protocol_version;
    union {
        uint8_t flag;
        struct {
            int : 1;    /**< unused */
            bool cleanstart: 1;    /**< cleansession flag */
            bool willflag: 1;            /**< willflag flag */
            unsigned int willQoS: 2;    /**< willflag QoS value */
            bool willRetain: 1;        /**< willflag retain setting */
            bool password: 1;            /**< 3.1 password */
            bool username: 1;            /**< 3.1 user name */
        } bits;
    } connect_flags;
    uint16_t keep_alive;
    MQTTProperties properties;

    // payload
    struct {
        MQTTLenString clientID,    /**< string client id */
        willTopic,    /**< willflag topic */
        willMsg,    /**< willflag payload */
        username,
        password;
        MQTTProperties willProperties; /**< willflag Properties */
    } payload;
} MQTT_Connect_t;

#define MQTT_Connect_t_INIT {0, 0, 0, "MQTT", 0, 0, 0,{0, 0, 0, NULL}, {{0, NULL}, {0, NULL}, {0, NULL}, {0, NULL}, {0, NULL}, {0, 0, 0, NULL}}};

int parse_mqtt_connect_message(char *message, size_t message_len, MQTT_Connect_t *connect);
int gen_mqtt_connect_message(char **message, size_t *message_len, MQTT_Connect_t *connect);
int free_mqtt_connect_message(MQTT_Connect_t *connect);
int modify_mqtt_connect_message(char **message, size_t *message_len);

#endif //MQTT_PROTOCOL_TEST_MQTTPARSER_H
