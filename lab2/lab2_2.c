#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>

typedef struct {
    volatile int VAR;
    sem_t readLock;
    sem_t writeLock;
    volatile int readers;
} shared_buffer;

shared_buffer *buffer;

void writer_process() {
    while(1) {
        sem_wait(&buffer->writeLock);
        if(buffer->VAR >= 10) {
            sem_post(&buffer->writeLock);
            break;
        }
        buffer->VAR++;
        printf("Writer PID %d writing value %d\n", getpid(), buffer->VAR);
        sem_post(&buffer->writeLock);
        sleep(1);
    }
}

void reader_process() {
    while(1) {
        sem_wait(&buffer->readLock);
        buffer->readers++;
        if(buffer->readers == 1) {
            sem_wait(&buffer->writeLock);
        }
        sem_post(&buffer->readLock);

        printf("Reader PID %d reading value %d\n", getpid(), buffer->VAR);
        if(buffer->VAR >= 10) {
            sem_wait(&buffer->readLock);
            buffer->readers--;
            if(buffer->readers == 0) {
                sem_post(&buffer->writeLock);
            }
            sem_post(&buffer->readLock);
            break;
        }

        sem_wait(&buffer->readLock);
        buffer->readers--;
        if(buffer->readers == 0) {
            sem_post(&buffer->writeLock);
        }
        sem_post(&buffer->readLock);
        sleep(1);
    }
}

int main() {
    int shm_fd = shm_open("/shm_buffer", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(shared_buffer));
    buffer = mmap(0, sizeof(shared_buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    buffer->VAR = 0;
    buffer->readers = 0;
    sem_init(&buffer->readLock, 1, 1);
    sem_init(&buffer->writeLock, 1, 1);

    pid_t wpid, rpid1, rpid2;

    wpid = fork();
    if(wpid == 0) {
        writer_process();
        exit(0);
    }

    rpid1 = fork();
    if(rpid1 == 0) {
        reader_process();
        exit(0);
    }

    rpid2 = fork();
    if(rpid2 == 0) {
        reader_process();
        exit(0);
    }

    waitpid(wpid, NULL, 0);
    waitpid(rpid1, NULL, 0);
    waitpid(rpid2, NULL, 0);

    sem_destroy(&buffer->readLock);
    sem_destroy(&buffer->writeLock);
    munmap(buffer, sizeof(shared_buffer));
    shm_unlink("/shm_buffer");

    return 0;
}
