CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
TARGET = mqtt_client
OBJS = client_mqtt.o mqtt.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

client_mqtt.o: client_mqtt.c mqtt.h
	$(CC) $(CFLAGS) -c client_mqtt.c

mqtt.o: mqtt.c mqtt.h
	$(CC) $(CFLAGS) -c mqtt.c

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run