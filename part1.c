#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>

int part1(void) {

    int file_desc[2]; // 0 - read, 1 - write 

    pipe(file_desc); // creating the pipe on the file descriptor array

    switch (fork()) {
        case -1: // pid < 0 => something wrong
            break;
        case 0: //child - should run ls / and write to file_desc[1]
            close(file_desc[0]); // close read since it will not use it.
            dup2(file_desc[1], STDOUT_FILENO); // redirecting  STDOUT (terminal output) of child to write-end of pipe.
            execlp("ls", "ls", "/", NULL); // first `ls` is the command to be executed, the second is what the program sees as argv[0], i.e. name of the program.
            perror("execlp ls / failed");
            exit(1);
        default: //parent - should run wc -l and read from file_desc[0]
            close(file_desc[1]); // closing write since it will not use it.
            dup2(file_desc[0], STDIN_FILENO);  // reads input from the read-end of the pipe instead of terminal output/user keyboard entries.
            wait(NULL); // waiting for child process termination.
            execlp("wc", "wc", "-l", NULL);
            perror("execlp wc -l failed");
            return 1;
    }
    return 0;
}

int main(void) {
    part1();

    return 0;
}