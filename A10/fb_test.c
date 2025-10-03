#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    int fd;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long screensize;
    uint32_t *fbmem;
    uint32_t color;
    int dummy = 0;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s 0xRRGGBB\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parsear el color en formato 0xRRGGBB
    color = (uint32_t) strtol(argv[1], NULL, 16);

    // Abrir framebuffer
    fd = open("/dev/fb0", O_RDWR);
    if (fd == -1) {
        perror("Error abriendo /dev/fb0");
        return EXIT_FAILURE;
    }

    // Obtener info de la pantalla
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo)) {
        perror("Error en FBIOGET_FSCREENINFO");
        close(fd);
        return EXIT_FAILURE;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Error en FBIOGET_VSCREENINFO");
        close(fd);
        return EXIT_FAILURE;
    }

    screensize = vinfo.yres_virtual * finfo.line_length;

    // Mapear memoria del framebuffer
    fbmem = (uint32_t *) mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if ((intptr_t)fbmem == -1) {
        perror("Error en mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    // Llenar la pantalla con el color recibido
    for (int y = 0; y < vinfo.yres; y++) {
        for (int x = 0; x < vinfo.xres; x++) {
            fbmem[y * (finfo.line_length / 4) + x] = color;
        }
    }

    // Sincronizar con VSync
    if (ioctl(fd, FBIO_WAITFORVSYNC, &dummy) == -1) {
        fprintf(stderr, "Advertencia: FBIO_WAITFORVSYNC no soportado (%s)\n", strerror(errno));
    }

    // Liberar memoria y cerrar
    munmap(fbmem, screensize);
    close(fd);

    return EXIT_SUCCESS;
}
