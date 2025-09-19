#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "common.h"

int main(int argc, char *argv[]) {
    char *buff;
    int ret;

    int shm_id = shmget(SHM_KEY, 4096, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(-1);
    }

    buff = shmat(shm_id, 0, 0);

    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    if (sem == SEM_FAILED) {
        exit(-1);
    }

    for (;;) {
        printf("Writer> ");
        char *str = fgets(buff, MAX_LEN, stdin);
        if (str == NULL)
            break;

        buff[strcspn(buff, "\n")] = '\0';

        sem_post(sem);
    }

    ret = shmdt(buff);
    if (ret == 0)
        printf("Memory detached\n");

    sem_close(sem);
    return 0;
}

