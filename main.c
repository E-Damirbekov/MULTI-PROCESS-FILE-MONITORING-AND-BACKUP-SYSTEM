#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include "scanner.h"
#include "backup.h"

/* Signal handler to perform clean exit on Ctrl+C */
void handle_sigint(int sig) {
    printf("\n[SIGINT] Termination signal %d caught. Exiting...\n", sig);
    exit(0);
}

/* * Entry point: Manages multi-processing, file scanning, and IPC logging.
 */
int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);

    if (argc < 2) {
        printf("Usage: ./monitor <source_directory>\n");
        return 1;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) { // Setup anonymous pipe for parent-child communication
        perror("pipe");
        return 1;
    }

    pid_t pid = fork(); // Create child process to isolate logging tasks

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) { // CHILD PROCESS: Logging specialized task
        close(pipefd[1]); // Close unused write end
        FILE *report = fopen("logs/report.txt", "a");
        char buffer[512];

        /* Listen to the pipe and log status updates received from the parent */
        while (read(pipefd[0], buffer, sizeof(buffer)) > 0) {
            fprintf(report, "[LOG]: %s\n", buffer);
            fflush(report);
        }

        fclose(report);
        close(pipefd[0]);
        exit(0);
    } else { // PARENT PROCESS: Scanning and management task
        close(pipefd[0]); // Close unused read end

        int count = 0;
        FileInfo *files = scan_directory(argv[1], &count);

        if (files != NULL && count > 0) {
            /* Process each found file: backup it and notify the child process */
            for (int i = 0; i < count; i++) {
                char log_msg[512];
                if (perform_backup(argv[1], files[i].name) == 0) {
                    sprintf(log_msg, "Success: %s backed up.", files[i].name);
                } else {
                    sprintf(log_msg, "Error: Failed to backup %s.", files[i].name);
                }
                /* Send task status to the child logger via pipe */
                write(pipefd[1], log_msg, strlen(log_msg) + 1); 
            }
            free(files); // Release memory allocated by the scanner module
        }

        close(pipefd[1]); // Closing write end signals EOF to the child process
        wait(NULL); // Reap child process to prevent zombie states
    }

    return 0;
}