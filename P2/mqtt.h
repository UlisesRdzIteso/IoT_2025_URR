#ifndef MQTT_H
#define MQTT_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// MQTT Control Packet Types
#define MQTT_CONNECT        0x10
#define MQTT_CONNACK        0x20
#define MQTT_PUBLISH        0x30
#define MQTT_PUBACK         0x40
#define MQTT_SUBSCRIBE      0x82
#define MQTT_SUBACK         0x90
#define MQTT_PINGREQ        0xC0
#define MQTT_PINGRESP       0xD0
#define MQTT_DISCONNECT     0xE0

// MQTT Protocol Name and Level
#define MQTT_PROTOCOL_NAME  "MQTT"
#define MQTT_PROTOCOL_LEVEL 0x04

// Connect Flags
#define MQTT_CLEAN_SESSION  0x02
#define MQTT_WILL_FLAG      0x04
#define MQTT_WILL_QOS_0     0x00
#define MQTT_WILL_QOS_1     0x08
#define MQTT_WILL_QOS_2     0x10
#define MQTT_WILL_RETAIN    0x20
#define MQTT_PASSWORD_FLAG  0x40
#define MQTT_USERNAME_FLAG  0x80

// Connection Return Codes
#define MQTT_CONN_ACCEPTED              0x00
#define MQTT_CONN_REFUSED_PROTOCOL      0x01
#define MQTT_CONN_REFUSED_IDENTIFIER    0x02
#define MQTT_CONN_REFUSED_SERVER        0x03
#define MQTT_CONN_REFUSED_CREDENTIALS   0x04
#define MQTT_CONN_REFUSED_UNAUTHORIZED  0x05

// Maximum buffer size
#define MQTT_MAX_PACKET_SIZE 256

// Structure for MQTT CONNECT packet
typedef struct {
    uint8_t fixed_header;
    uint8_t remaining_length;
    uint8_t protocol_name_len_msb;
    uint8_t protocol_name_len_lsb;
    char protocol_name[4];
    uint8_t protocol_level;
    uint8_t connect_flags;
    uint8_t keep_alive_msb;
    uint8_t keep_alive_lsb;
} __attribute__((packed)) mqtt_connect_fixed_t;

// Structure for MQTT CONNACK packet
typedef struct {
    uint8_t fixed_header;
    uint8_t remaining_length;
    uint8_t session_present;
    uint8_t return_code;
} __attribute__((packed)) mqtt_connack_t;

// Structure for MQTT PINGREQ packet
typedef struct {
    uint8_t fixed_header;
    uint8_t remaining_length;
} __attribute__((packed)) mqtt_pingreq_t;

// Structure for MQTT PINGRESP packet
typedef struct {
    uint8_t fixed_header;
    uint8_t remaining_length;
} __attribute__((packed)) mqtt_pingresp_t;

// QoS Levels
#define QOS_0   0x00
#define QOS_1   0x01
#define QOS_2   0x02

// Function declarations
int mqtt_create_connect(uint8_t *buffer, const char *client_id, uint16_t keep_alive);
int mqtt_create_pingreq(uint8_t *buffer);
int mqtt_create_publish(uint8_t *buffer, const char *topic, const char *payload, uint8_t qos, uint8_t retain, uint16_t *packet_id);
int mqtt_create_subscribe(uint8_t *buffer, const char *topic, uint8_t qos, uint16_t packet_id);
int mqtt_parse_connack(uint8_t *buffer, uint8_t *session_present, uint8_t *return_code);
int mqtt_parse_pingresp(uint8_t *buffer);
int mqtt_parse_suback(uint8_t *buffer, uint16_t *packet_id, uint8_t *return_code);
int mqtt_parse_publish(uint8_t *buffer, int buffer_len, char *topic, int topic_size, char *payload, int payload_size);
void mqtt_print_packet(uint8_t *buffer, int length);

#endif