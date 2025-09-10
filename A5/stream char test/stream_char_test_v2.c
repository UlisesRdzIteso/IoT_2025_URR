#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define LEN 80

void stream_char_test();
void stream_string_test();

int main(int argc, char * argv[]) {
    printf("Hi from %s\n", argv[0]);

    int stdin_bup = dup(0);
    FILE *fstdin_bup = fdopen(stdin_bup, "r");

    if (!fstdin_bup) {
        perror("fdopen error");
        return -1;
    }

    stream_string_test();

    return 0;
}

void stream_char_test() {
    FILE * fout, * fin;
    int c;

    fout = fopen("test_file_char.txt", "w");
    if (fout == NULL) {
        perror("Error abriendo archivo");
        exit(-1);
    }

    printf("File descriptor of test_file_char.txt is %d\n", fileno(fout));
    printf("Escribe texto:\n");

    fin = fdopen(0, "r");

    while ((c = fgetc(fin)) != EOF) {
        fputc(c, fout);
    }

    fclose(fout);
}

void stream_string_test() {
    FILE * fout, * fin;
    char line[LEN + 1];

    fout = fopen("test_file_string.txt", "w");
    if (fout == NULL) {
        perror("Error abriendo archivo");
        exit(-1);
    }

    printf("File descriptor of test_file_string.txt is %d\n", fileno(fout));
    printf("Escribe texto:\n");

    fin = fdopen(0, "r");

    while (fgets(line, LEN, fin) != NULL) {
        fputs(line, fout);
    }

    fclose(fout);
}

