CC = gcc
CFLAGS = -Wall

TARGET = led_random

all: $(TARGET)

$(TARGET): led_random.c
	$(CC) $(CFLAGS) led_random.c -o $(TARGET)

clean:
	rm -f $(TARGET)

