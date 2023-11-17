#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>

#include <mqueue.h> // POSIX message queue API
#include <fcntl.h> // "file control" - manipulating file descriptors


#define QUEUE_NAME "/my_queue"
#define MAX_SIZE    1024
#define MAX_MSG     10
#define FILE_PATH   "file.txt"

// Process A --> [m_0, m_1, ..., m_n] <-- Process B

void msg_producer() { // child process
    mqd_t mq; // message queue data type
    char buffer[MAX_SIZE];
    
    struct mq_attr attr; // setting initial message queue attributes
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MAX_SIZE;

    // read pointer stream
    FILE *file_ptr = fopen(FILE_PATH, "r");
    if (file_ptr == NULL) {
        perror("File open to read error");
        exit(1);
    }  

    //printf("msg producer\n");
    // if queue exists, open it write only. Else create a new one based and use the mq attributes specified
    mq = mq_open(QUEUE_NAME, O_WRONLY | O_CREAT, 0644, &attr); 
    if (mq == -1) { 
        perror("Producer mq_open");
        exit(1);
    }

    // reading MAX_SIZE from file_ptr into buffer. Then use mq_send to send buffer to the created queue, mq.
    while (fgets(buffer, MAX_SIZE, file_ptr)) {
        //printf("Sending message: %s", buffer);
        if (mq_send(mq, buffer, strlen(buffer), 0) == -1) {
            perror("mq_send error");
            break;
        }
    }

    fclose(file_ptr);
    mq_close(mq);

}

void msg_consumer() { // parent process
    mqd_t mq; // data type for message queue descriptors
    char buffer[MAX_SIZE];
    int words = 0;


    // mq = open queue in read only mode
    mq = mq_open(QUEUE_NAME, O_RDONLY);
    if (mq == -1) { // error
        perror("mq_open error: ");
        exit(1);
    }

    // printf("msg_producer\n");

    // receive message of MAX_SIZE from mq into buffer
    while (mq_receive(mq, buffer, MAX_SIZE, NULL) > 0) {
        //printf("Received message: %s", buffer);
        char *token = strtok(buffer, " \n"); // tokenize the string buffer based on space char or new line
        while (token != NULL) {
            //printf("Words = %s\n",token);
            words++;
            token = strtok(NULL, " \n");
        }
        printf("Total words: %d\n", words);
    }

    //printf("Total words: %d\n", words);
    mq_close(mq);
    mq_unlink(QUEUE_NAME);
}
 
// simple parent-child format. 
int part2(void) {
    switch (fork()) {
        case -1: 
            perror("fork error");
            exit(1);
            break;
        case 0: // child
            msg_producer();
            break;
        default:  // parent
            wait(NULL);
            msg_consumer();
            break;
    }
    
    return 0;
}

int main(void) {
    part2();

    return 0;
}
