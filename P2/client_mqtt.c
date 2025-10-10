#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "mqtt.h"

#define PORT 1883
#define KEEP_ALIVE 60
#define CLIENT_ID "mqtt_client_001"

pthread_mutex_t socketMutex = PTHREAD_MUTEX_INITIALIZER;
int g_connected = 0;
int g_sock = 0;
uint16_t g_packet_id = 1;

uint16_t get_next_packet_id(void)
{
    return g_packet_id++;
}

void* receive_thread(void* arg)
{
    uint8_t buffer[MQTT_MAX_PACKET_SIZE];
    ssize_t bytes_read;
    uint8_t session_present, return_code;
    uint16_t packet_id;
    char topic[128];
    char payload[128];
    
    while (1) {
        memset(buffer, 0, MQTT_MAX_PACKET_SIZE);
        bytes_read = recv(g_sock, buffer, MQTT_MAX_PACKET_SIZE, 0);
        
        if (bytes_read <= 0) {
            printf("Connection closed by broker\n");
            g_connected = 0;
            break;
        }
        
        printf("\n--- Received packet ---\n");
        mqtt_print_packet(buffer, bytes_read);
        
        switch (buffer[0] & 0xF0) {
            case MQTT_CONNACK:
                if (mqtt_parse_connack(buffer, &session_present, &return_code) == 0) {
                    if (return_code == MQTT_CONN_ACCEPTED) {
                        g_connected = 1;
                        printf("Session Present: %d\n", session_present);
                    }
                }
                break;
                
            case MQTT_PINGRESP:
                mqtt_parse_pingresp(buffer);
                break;
                
            case MQTT_SUBACK:
                mqtt_parse_suback(buffer, &packet_id, &return_code);
                break;
                
            case MQTT_PUBLISH:
                mqtt_parse_publish(buffer, bytes_read, topic, sizeof(topic), payload, sizeof(payload));
                break;
                
            default:
                printf("Received packet type: 0x%02X\n", buffer[0]);
                break;
        }
    }
    
    return NULL;
}

void* ping_thread(void* arg)
{
    uint8_t ping_buffer[2];
    int ping_len;
    int ping_interval = KEEP_ALIVE / 2;
    
    sleep(5);
    
    while (1) {
        sleep(ping_interval);
        
        if (g_connected) {
            ping_len = mqtt_create_pingreq(ping_buffer);
            
            pthread_mutex_lock(&socketMutex);
            printf("\n--- Sending PINGREQ ---\n");
            mqtt_print_packet(ping_buffer, ping_len);
            
            if (send(g_sock, ping_buffer, ping_len, 0) < 0) {
                printf("Error sending PINGREQ\n");
                g_connected = 0;
            }
            pthread_mutex_unlock(&socketMutex);
        }
    }
    
    return NULL;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in serv_addr;
    uint8_t mqtt_buffer[MQTT_MAX_PACKET_SIZE];
    int packet_len;
    pthread_t recv_tid, ping_tid;
    char user_input[100];

    if ((g_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("192.168.1.1");
    
    printf("Connecting to MQTT broker at port %d...\n", PORT);
    if (connect(g_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        close(g_sock);
        return -1;
    }
    printf("TCP connection established\n");
    
    packet_len = mqtt_create_connect(mqtt_buffer, CLIENT_ID, KEEP_ALIVE);
    printf("\n--- Sending CONNECT packet ---\n");
    printf("Client ID: %s\n", CLIENT_ID);
    printf("Keep Alive: %d seconds\n", KEEP_ALIVE);
    mqtt_print_packet(mqtt_buffer, packet_len);
    
    if (send(g_sock, mqtt_buffer, packet_len, 0) < 0) {
        printf("Error sending CONNECT packet\n");
        close(g_sock);
        return -1;
    }

    if (pthread_create(&recv_tid, NULL, receive_thread, NULL) != 0) {
        printf("Failed to create receive thread\n");
        close(g_sock);
        return -1;
    }
    
    if (pthread_create(&ping_tid, NULL, ping_thread, NULL) != 0) {
        printf("Failed to create ping thread\n");
        close(g_sock);
        return -1;
    }
    
    printf("\n=== MQTT Client Started ===\n");
    printf("Commands:\n");
    printf("  ping                          - Send PINGREQ manually\n");
    printf("  pub <topic> <message>         - Publish message to topic\n");
    printf("  sub <topic>                   - Subscribe to topic\n");
    printf("  status                        - Show connection status\n");
    printf("  quit                          - Exit application\n");
    printf("===========================\n\n");
    
    while (1) {
        printf("> ");
        if (fgets(user_input, sizeof(user_input), stdin) == NULL) {
            break;
        }
        
        user_input[strcspn(user_input, "\n")] = 0;
        
        if (strcmp(user_input, "quit") == 0) {
            printf("Disconnecting...\n");
            break;
        }
        else if (strcmp(user_input, "ping") == 0) {
            if (g_connected) {
                packet_len = mqtt_create_pingreq(mqtt_buffer);
                pthread_mutex_lock(&socketMutex);
                printf("\n--- Sending PINGREQ (manual) ---\n");
                mqtt_print_packet(mqtt_buffer, packet_len);
                send(g_sock, mqtt_buffer, packet_len, 0);
                pthread_mutex_unlock(&socketMutex);
            } else {
                printf("Not connected to broker\n");
            }
        }
        else if (strncmp(user_input, "pub ", 4) == 0) {
            if (g_connected) {
                char *topic = strtok(user_input + 4, " ");
                char *message = strtok(NULL, "");
                
                if (topic && message) {
                    uint16_t pid = get_next_packet_id();
                    packet_len = mqtt_create_publish(mqtt_buffer, topic, message, QOS_0, 0, &pid);
                    
                    pthread_mutex_lock(&socketMutex);
                    printf("\n--- Publishing to '%s' ---\n", topic);
                    printf("Message: %s\n", message);
                    mqtt_print_packet(mqtt_buffer, packet_len);
                    send(g_sock, mqtt_buffer, packet_len, 0);
                    pthread_mutex_unlock(&socketMutex);
                } else {
                    printf("Usage: pub <topic> <message>\n");
                }
            } else {
                printf("Not connected to broker\n");
            }
        }
        else if (strncmp(user_input, "sub ", 4) == 0) {
            if (g_connected) {
                char *topic = user_input + 4;
                
                if (strlen(topic) > 0) {
                    uint16_t pid = get_next_packet_id();
                    packet_len = mqtt_create_subscribe(mqtt_buffer, topic, QOS_0, pid);
                    
                    pthread_mutex_lock(&socketMutex);
                    printf("\n--- Subscribing to '%s' ---\n", topic);
                    printf("Packet ID: %d\n", pid);
                    mqtt_print_packet(mqtt_buffer, packet_len);
                    send(g_sock, mqtt_buffer, packet_len, 0);
                    pthread_mutex_unlock(&socketMutex);
                } else {
                    printf("Usage: sub <topic>\n");
                }
            } else {
                printf("Not connected to broker\n");
            }
        }
        else if (strcmp(user_input, "status") == 0) {
            printf("Connection status: %s\n", g_connected ? "CONNECTED" : "DISCONNECTED");
        }
        else if (strlen(user_input) > 0) {
            printf("Unknown command: %s\n", user_input);
        }
    }
    
    close(g_sock);
    printf("Connection closed\n");
    
    return 0;
}