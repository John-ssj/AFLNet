//
// Created by 史导的Mac on 2023/4/3.
//

#include <stdlib.h>
#include <string.h>

#include "MQTTParser.h"
#include "../alloc-inl.h"

int parse_mqtt_connect_message(char *message, size_t message_len, MQTT_Connect_t *connect) {
    char *curdata = message;
    char *enddata = &message[message_len];
    // Parse the fixed header
    connect->fixed_header = *curdata++;
    if ((connect->fixed_header & 0xf0) != 0x10) { return -1; }

    // Parse the remaining length field
    int l_byte = MQTTPacket_decodeBuf(curdata, &connect->remaining_length);
    curdata += l_byte;

    // Parse the variable header and payload
    connect->protocol_name_len = (*curdata << 8) | *(curdata + 1);
    curdata += 2;
    if (connect->protocol_name_len > 4) {
        return -1; // protocol_name_len error;
    }
    memcpy(connect->protocol_name, curdata, connect->protocol_name_len);
    curdata += connect->protocol_name_len;
    connect->protocol_version = *curdata++;
    connect->connect_flags.flag = *curdata++;
    connect->keep_alive = (*curdata << 8) | *(curdata + 1);
    curdata += 2;

    // Parse the property_list field
    if (connect->protocol_version >= 5 && enddata - curdata > 0) {
        int ret = MQTTProperties_read(&connect->properties, &curdata, enddata);
        if (ret != 1) { return -1; } // failed to parse property_list field
    }

    // parse the payload
    if (enddata - curdata > 0) {

        // ClientID
        if (enddata - curdata >= 2) {
            u_int16_t ClientId_length = (*curdata << 8) | *(curdata + 1);
            curdata += 2;
            if (enddata - curdata >= ClientId_length) {
                char *temp = malloc(ClientId_length);
                if (temp) {
                    memcpy(temp, curdata, ClientId_length);
                    curdata += ClientId_length;
                    connect->payload.clientID.len = ClientId_length;
                    connect->payload.clientID.data = temp;
                }
            }
        }

        // willProperties
        if (connect->protocol_version >= 5 && connect->connect_flags.bits.willflag && enddata - curdata > 0) {
            int ret = MQTTProperties_read(&connect->payload.willProperties, &curdata, enddata);
            if (ret != 1) { return -1; } // failed to parse property_list field
        }

        // WillTopic
        if (connect->connect_flags.bits.willflag && enddata - curdata >= 2) {
            u_int16_t willTopic_length = (*curdata << 8) | *(curdata + 1);
            curdata += 2;
            if (enddata - curdata >= willTopic_length) {
                char *temp = malloc(willTopic_length);
                if (temp) {
                    memcpy(temp, curdata, willTopic_length);
                    curdata += willTopic_length;
                    connect->payload.willTopic.len = willTopic_length;
                    connect->payload.willTopic.data = temp;
                }
            }
        }

        // WillMsg
        if (connect->connect_flags.bits.willflag && enddata - curdata >= 2) {
            u_int16_t willMsg_length = (*curdata << 8) | *(curdata + 1);
            curdata += 2;
            if (enddata - curdata >= willMsg_length) {
                char *temp = malloc(willMsg_length);
                if (temp) {
                    memcpy(temp, curdata, willMsg_length);
                    curdata += willMsg_length;
                    connect->payload.willMsg.len = willMsg_length;
                    connect->payload.willMsg.data = temp;
                }
            }
        }

        // username
        if (connect->connect_flags.bits.username && enddata - curdata >= 2) {
            u_int16_t username_length = (*curdata << 8) | *(curdata + 1);
            curdata += 2;
            if (enddata - curdata >= username_length) {
                char *temp = malloc(username_length);
                if (temp) {
                    memcpy(temp, curdata, username_length);
                    curdata += username_length;
                    connect->payload.username.len = username_length;
                    connect->payload.username.data = temp;
                }
            }
        }

        // password
        if (connect->connect_flags.bits.password && enddata - curdata >= 2) {
            u_int16_t password_length = (*curdata << 8) | *(curdata + 1);
            curdata += 2;
            if (enddata - curdata >= password_length) {
                char *temp = malloc(password_length);
                if (temp) {
                    memcpy(temp, curdata, password_length);
                    curdata += password_length;
                    connect->payload.password.len = password_length;
                    connect->payload.password.data = temp;
                }
            }
        }
    }

    return 0; // success
}


int gen_mqtt_connect_message(char **message, size_t *message_len, MQTT_Connect_t *connect) {
    *message_len = 1 + MQTTPacket_VBIlen(connect->remaining_length) + connect->remaining_length;
    *message = (char *)ck_alloc_nozero(*message_len);
//    *message = (char *)alloca(*message_len);
    char *current = *message;
    *current++ = connect->fixed_header;
    current += MQTTPacket_encode(current, connect->remaining_length);
    writeInt(&current, connect->protocol_name_len);
    memcpy(current, connect->protocol_name, connect->protocol_name_len);
    current += connect->protocol_name_len;
    *current++ = connect->protocol_version;
    *current++ = connect->connect_flags.flag;
    writeInt(&current, connect->keep_alive);
    if (connect->protocol_version >= 5) {
        MQTTProperties_write(&current, &connect->properties);
    }
    writeMQTTLenString(&current, connect->payload.clientID);
    if (connect->protocol_version >= 5 && connect->connect_flags.bits.willflag) {
        MQTTProperties_write(&current, &connect->payload.willProperties);
    }
    if (connect->connect_flags.bits.willflag) {
        writeMQTTLenString(&current, connect->payload.willTopic);
        writeMQTTLenString(&current, connect->payload.willMsg);
    }
    if (connect->connect_flags.bits.username) {
        writeMQTTLenString(&current, connect->payload.username);
    }
    if (connect->connect_flags.bits.password) {
        writeMQTTLenString(&current, connect->payload.password);
    }
    return 0; // success
}


int free_mqtt_connect_message(MQTT_Connect_t *connect) {
    if (connect == NULL) { return 0; }

    MQTTProperties_free(&connect->properties);
    MQTTProperties_free(&connect->payload.willProperties);
    if (connect->payload.clientID.data != NULL) { free(connect->payload.clientID.data); }
    if (connect->payload.willTopic.data != NULL) { free(connect->payload.willTopic.data); }
    if (connect->payload.willMsg.data != NULL) { free(connect->payload.willMsg.data); }
    if (connect->payload.username.data != NULL) { free(connect->payload.username.data); }
    if (connect->payload.password.data != NULL) { free(connect->payload.password.data); }
    return 0;
}

int update_mqtt_connect_message_length(MQTT_Connect_t *connect) {
    // Variable header
    int connect_length_withoutFixheader = 2 + 4 + 1 + 1 + 2 + MQTTProperties_len(&connect->properties);
    // payload
    connect_length_withoutFixheader += 2 + connect->payload.clientID.len;
    if (connect->connect_flags.bits.willflag)
        connect_length_withoutFixheader = 2 + MQTTProperties_len(&connect->payload.willProperties);
    if (connect->connect_flags.bits.willflag) connect_length_withoutFixheader = 2 + connect->payload.willTopic.len;
    if (connect->connect_flags.bits.willflag) connect_length_withoutFixheader = 2 + connect->payload.willMsg.len;
    if (connect->connect_flags.bits.username) connect_length_withoutFixheader = 2 + connect->payload.username.len;
    if (connect->connect_flags.bits.password) connect_length_withoutFixheader = 2 + connect->payload.password.len;

    connect->remaining_length = connect_length_withoutFixheader;
    return 0;
}

int modify_mqtt_connect_message(char **message, size_t *message_len) {
    MQTT_Connect_t connect = MQTT_Connect_t_INIT;
    if (parse_mqtt_connect_message(*message, *message_len, &connect) != 0) { return -1; }

    // add new properties
    MQTTProperty userProp_1 = {.identifier=MQTTPROPERTY_CODE_USER_PROPERTY, .value={.data={4, "name"}, .value={4,
                                                                                                               "john"}}};
    MQTTProperties_add(&connect.properties, &userProp_1);
    MQTTProperty userProp_2 = {.identifier=MQTTPROPERTY_CODE_USER_PROPERTY, .value={.data={4, "data"}, .value={4,
                                                                                                               "0405"}}};
    MQTTProperties_add(&connect.properties, &userProp_2);
    update_mqtt_connect_message_length(&connect);

    if(*message != NULL) {
        ck_free(*message);
//        free(*message);
    }
    gen_mqtt_connect_message(message, message_len, &connect);
    free_mqtt_connect_message(&connect);
    return 0;
}