#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "scanner.h"
#include "backup.h"

/* Signal handler for clean exit on SIGINT */
void handle_sigint(int sig) {
    printf("\n[SIGINT] caught. Exiting...\n");
    exit(0);
}

/* Alert function for urgent situations */
void create_urgent_alert(const char* type, const char* message) {
    char filename[256];
    sprintf(filename, "alerts/%s.alert", type);
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "--- URGENT ALERT ---\n");
        fprintf(f, "Status: %s\n", message);
        fprintf(f, "--------------------\n");
        fclose(f);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);

    if (argc < 2) {
        printf("Usage: ./monitor <source_directory>\n");
        return 1;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) { // CHILD PROCESS: Logger
        close(pipefd[1]); 
        FILE *report = fopen("logs/report.txt", "a");
        char buffer[512];

        /* Simple logging without paths or timestamps */
        while (read(pipefd[0], buffer, sizeof(buffer)) > 0) {
            fprintf(report, "[LOG]: %s\n", buffer);
            fflush(report);
        }

        fclose(report);
        close(pipefd[0]);
        exit(0);
    } else { // PARENT PROCESS: Scanner & Backup
        close(pipefd[0]); 

        int count = 0;
        int success_count = 0;
        FileInfo *files = scan_directory(argv[1], &count);

        if (count == 0) {
            /* Alert for empty source folder */
            create_urgent_alert("EMPTY", "No files found in source directory.");
            write(pipefd[1], "Warning: No files found.", 25);
        } else if (files != NULL) {
            for (int i = 0; i < count; i++) {
                char log_msg[1024];
                if (perform_backup(argv[1], files[i].name) == 0) {
                    sprintf(log_msg, "Success: %s copied.", files[i].name);
                    success_count++;
                } else {
                    /* Alert for failed backup */
                    sprintf(log_msg, "Critical: %s failed.", files[i].name);
                    create_urgent_alert("FAILURE", log_msg);
                }
                write(pipefd[1], log_msg, strlen(log_msg) + 1); 
            }
            free(files);
        }

        close(pipefd[1]); 
        wait(NULL); 
        printf("Done. Check logs/ and alerts/.\n");
    }

    return 0;
}