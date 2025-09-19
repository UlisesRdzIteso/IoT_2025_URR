#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "common.h"

int main(int argc, char *argv[]) {
    char *buff;
    int ret;

    int shm_id = shmget(SHM_KEY, 0, 0);
    if (shm_id == -1) {
        perror("shmget");
        exit(-1);
    }

    buff = shmat(shm_id, 0, 0);

    sem_t *sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        exit(-1);
    }

    for (;;) {
        sem_wait(sem);

        printf("Reader> %s\n", buff);
    }

    ret = shmdt(buff);
    if (ret == 0)
        printf("Memory detached\n");

    sem_close(sem);
    return 0;
}

