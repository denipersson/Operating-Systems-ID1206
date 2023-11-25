#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock;
int buffer = 0;

// accessed by each thread to ++ the buffer
void *thread_function(void *arg) {
    int times_changed = 0;

    while(1) {
        pthread_mutex_lock(&lock);
        if(buffer < 15){
            printf("TID: %ld, PID: %d, Buffer: %d\n", (long)pthread_self(), getpid(), buffer);
            buffer++;
            times_changed++;
        } else {
            pthread_mutex_unlock(&lock);
            break;
        }
         pthread_mutex_unlock(&lock);
    }

    int *result = malloc(sizeof(int));
    *result = times_changed;
    return result;
}

int main() {
    pthread_t thread1, thread2, thread3;

    pthread_mutex_init(&lock, NULL);

    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);
    pthread_create(&thread3, NULL, thread_function, NULL);

    void *t_result1;
    void *t_result2;
    void *t_result3;

    pthread_join(thread1, &t_result1);
    pthread_join(thread2, &t_result2);
    pthread_join(thread3, &t_result3);

    printf("precess 1 changed buffer %d times\n", *((int *)t_result1));
    printf("precess 2 changed buffer %d times\n", *((int *)t_result2));
    printf("precess 3 changed buffer %d times\n", *((int *)t_result3));

    free(t_result1); 
    free(t_result2); 
    free(t_result3); 

    pthread_mutex_destroy(&lock);
    return 0;
}
