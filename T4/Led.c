#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define LED_PATH "/sys/class/leds/gpio-led/brightness"

int main(void) {
    FILE *urandom, *led;
    unsigned char value;

    urandom = fopen("/dev/urandom", "rb");
    if (urandom == NULL) {
        perror("No se pudo abrir /dev/urandom");
        return 1;
    }

    while (1) {
        // Leer 1 byte
        if (fread(&value, 1, 1, urandom) != 1) {
            perror("Error al leer /dev/urandom");
            break;
        }

        led = fopen(LED_PATH, "w");
        if (led == NULL) {
            perror("No se pudo abrir LED");
            break;
        }

        if (value > 128) {
            fputs("1", led);  // Enciende LED
        } else {
            fputs("0", led);  // Apaga LED
        }
        fflush(led);
        fclose(led);

        sleep(1);  // Esperar 1 segundo
    }

    fclose(urandom);
    return 0;
}
