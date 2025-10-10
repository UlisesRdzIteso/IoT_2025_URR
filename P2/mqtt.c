#include "mqtt.h"

int mqtt_create_connect(uint8_t *buffer, const char *client_id, uint16_t keep_alive)
{
    int index = 0;
    uint16_t client_id_len = strlen(client_id);
    uint8_t remaining_length;
    
    // Calculate remaining length
    // 10 bytes (variable header) + 2 bytes (client ID length) + client ID length
    remaining_length = 10 + 2 + client_id_len;
    
    // Fixed Header
    buffer[index++] = MQTT_CONNECT;
    buffer[index++] = remaining_length;
    
    // Variable Header
    // Protocol Name Length (MSB, LSB)
    buffer[index++] = 0x00;
    buffer[index++] = 0x04;
    
    // Protocol Name "MQTT"
    buffer[index++] = 'M';
    buffer[index++] = 'Q';
    buffer[index++] = 'T';
    buffer[index++] = 'T';
    
    // Protocol Level
    buffer[index++] = MQTT_PROTOCOL_LEVEL;
    
    // Connect Flags
    buffer[index++] = MQTT_CLEAN_SESSION;
    
    // Keep Alive
    buffer[index++] = (keep_alive >> 8) & 0xFF;
    buffer[index++] = keep_alive & 0xFF;
    
    // Payload
    // Client ID Length
    buffer[index++] = (client_id_len >> 8) & 0xFF;
    buffer[index++] = client_id_len & 0xFF;
    
    // Client ID
    memcpy(&buffer[index], client_id, client_id_len);
    index += client_id_len;
    
    return index;
}

int mqtt_create_pingreq(uint8_t *buffer)
{
    buffer[0] = MQTT_PINGREQ;
    buffer[1] = 0x00;
    return 2;
}

int mqtt_parse_connack(uint8_t *buffer, uint8_t *session_present, uint8_t *return_code)
{
    if (buffer[0] != MQTT_CONNACK) {
        printf("Error: Expected CONNACK packet (0x20), got 0x%02X\n", buffer[0]);
        return -1;
    }
    
    if (buffer[1] != 0x02) {
        printf("Error: Invalid CONNACK remaining length\n");
        return -1;
    }
    
    *session_present = buffer[2] & 0x01;
    *return_code = buffer[3];
    
    switch (*return_code) {
        case MQTT_CONN_ACCEPTED:
            printf("MQTT: Connection Accepted\n");
            break;
        case MQTT_CONN_REFUSED_PROTOCOL:
            printf("MQTT: Connection Refused - Unacceptable Protocol Version\n");
            break;
        case MQTT_CONN_REFUSED_IDENTIFIER:
            printf("MQTT: Connection Refused - Identifier Rejected\n");
            break;
        case MQTT_CONN_REFUSED_SERVER:
            printf("MQTT: Connection Refused - Server Unavailable\n");
            break;
        case MQTT_CONN_REFUSED_CREDENTIALS:
            printf("MQTT: Connection Refused - Bad Username/Password\n");
            break;
        case MQTT_CONN_REFUSED_UNAUTHORIZED:
            printf("MQTT: Connection Refused - Not Authorized\n");
            break;
        default:
            printf("MQTT: Connection Refused - Unknown Error Code: %d\n", *return_code);
            break;
    }
    
    return 0;
}

int mqtt_parse_pingresp(uint8_t *buffer)
{
    if (buffer[0] != MQTT_PINGRESP) {
        printf("Error: Expected PINGRESP packet (0xD0), got 0x%02X\n", buffer[0]);
        return -1;
    }
    
    if (buffer[1] != 0x00) {
        printf("Error: Invalid PINGRESP remaining length\n");
        return -1;
    }
    
    printf("MQTT: PINGRESP received\n");
    return 0;
}


int mqtt_create_publish(uint8_t *buffer, const char *topic, const char *payload, uint8_t qos, uint8_t retain, uint16_t *packet_id)
{
    int index = 0;
    uint16_t topic_len = strlen(topic);
    uint16_t payload_len = strlen(payload);
    uint8_t remaining_length;
    uint8_t flags = 0;
    
    // Calculate remaining length
    remaining_length = 2 + topic_len + payload_len;  // Topic length (2) + topic + payload
    if (qos > 0) {
        remaining_length += 2;
    }
    
    // Fixed Header
    flags = (qos << 1) | (retain ? 0x01 : 0x00);
    buffer[index++] = MQTT_PUBLISH | flags;
    buffer[index++] = remaining_length;
    
    // Variable Header
    buffer[index++] = (topic_len >> 8) & 0xFF;
    buffer[index++] = topic_len & 0xFF;
    
    // Topic
    memcpy(&buffer[index], topic, topic_len);
    index += topic_len;
    
    // Packet ID
    if (qos > 0) {
        buffer[index++] = (*packet_id >> 8) & 0xFF;
        buffer[index++] = *packet_id & 0xFF;
    }
    
    // Payload
    memcpy(&buffer[index], payload, payload_len);
    index += payload_len;
    
    return index;
}

int mqtt_create_subscribe(uint8_t *buffer, const char *topic, uint8_t qos, uint16_t packet_id)
{
    int index = 0;
    uint16_t topic_len = strlen(topic);
    uint8_t remaining_length;
    
    // 2 (packet ID) + 2 (topic length) + topic length + 1 (QoS)
    remaining_length = 2 + 2 + topic_len + 1;
    
    // Fixed Header
    buffer[index++] = MQTT_SUBSCRIBE;
    buffer[index++] = remaining_length;
    
    // Variable Header - Packet Identifier
    buffer[index++] = (packet_id >> 8) & 0xFF;
    buffer[index++] = packet_id & 0xFF;
    
    // Payload
    buffer[index++] = (topic_len >> 8) & 0xFF;
    buffer[index++] = topic_len & 0xFF;
    
    // Topic Filter
    memcpy(&buffer[index], topic, topic_len);
    index += topic_len;
    
    // Requested QoS
    buffer[index++] = qos;
    
    return index;
}

int mqtt_parse_suback(uint8_t *buffer, uint16_t *packet_id, uint8_t *return_code)
{
    if (buffer[0] != MQTT_SUBACK) {
        printf("Error: Expected SUBACK packet (0x90), got 0x%02X\n", buffer[0]);
        return -1;
    }
    
    // Extract packet ID
    *packet_id = (buffer[2] << 8) | buffer[3];
    
    // Extract return code
    *return_code = buffer[4];
    
    printf("MQTT: SUBACK received - Packet ID: %d, Return Code: 0x%02X\n", *packet_id, *return_code);
    
    if (*return_code == 0x80) {
        printf("      Subscription FAILED\n");
    } else {
        printf("      Subscription SUCCESS - Granted QoS: %d\n", *return_code);
    }
    
    return 0;
}

int mqtt_parse_publish(uint8_t *buffer, int buffer_len, char *topic, int topic_size, char *payload, int payload_size)
{
    if ((buffer[0] & 0xF0) != MQTT_PUBLISH) {
        printf("Error: Expected PUBLISH packet (0x3X), got 0x%02X\n", buffer[0]);
        return -1;
    }
    
    int index = 2;
    
    // Extract topic length
    uint16_t topic_len = (buffer[index] << 8) | buffer[index + 1];
    index += 2;
    
    // Extract topic
    if (topic_len >= topic_size) {
        printf("Error: Topic buffer too small\n");
        return -1;
    }
    memcpy(topic, &buffer[index], topic_len);
    topic[topic_len] = '\0';
    index += topic_len;
    
    // Check QoS to see if packet ID is present
    uint8_t qos = (buffer[0] >> 1) & 0x03;
    if (qos > 0) {
        index += 2;
    }
    
    // Extract payload
    int payload_len = buffer_len - index;
    if (payload_len >= payload_size) {
        printf("Error: Payload buffer too small\n");
        return -1;
    }
    memcpy(payload, &buffer[index], payload_len);
    payload[payload_len] = '\0';
    
    printf("MQTT: PUBLISH received - Topic: '%s', Payload: '%s'\n", topic, payload);
    
    return 0;
}

void mqtt_print_packet(uint8_t *buffer, int length)
{
    printf("Packet [%d bytes]: ", length);
    for (int i = 0; i < length; i++) {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}