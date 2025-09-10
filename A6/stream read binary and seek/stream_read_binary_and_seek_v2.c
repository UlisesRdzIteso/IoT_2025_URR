#include <stdio.h>
#include <stdlib.h>

#define LEN 80
#define ELEMENTS 3

typedef struct {
    int id;
    float data;
    char name[LEN];
} sAnalogData_t;

void fread_reverse_test() {
    FILE *fin = fopen("AnalogData.bin", "rb");
    if (!fin) {
        perror("Error al abrir archivo");
        exit(1);
    }

    sAnalogData_t tmp;

    for (int i = ELEMENTS - 1; i >= 0; i--) {
       
        fseek(fin, i * sizeof(sAnalogData_t), SEEK_SET);

        if (fread(&tmp, sizeof(sAnalogData_t), 1, fin) == 1) {
            printf("id=%d, data=%.2f, name=%s\n", tmp.id, tmp.data, tmp.name);
        } else {
            perror("Error al leer estructura");
        }
    }

    fclose(fin);
}

int main(void) {
    fread_reverse_test();
    return 0;
}


