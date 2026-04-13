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

/* Signal handler for clean exit on SIGINT (Ctrl+C) */
void handle_sigint(int sig) {
    printf("\n[SIGINT] Termination signal %d caught. Exiting...\n", sig);
    exit(0);
}

/* Helper function to get a formatted timestamp for logging */
void get_timestamp(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", t);
}

/* Creates specific alert files in the alerts/ directory for summaries or errors */
void create_system_alert(const char* type, const char* message, const char* folder, int count) {
    char filename[256];
    sprintf(filename, "alerts/%s.alert", type);
    FILE *f = fopen(filename, "w");
    if (f) {
        char ts[32];
        get_timestamp(ts);
        fprintf(f, "--- SYSTEM ALERT: %s ---\n", type);
        fprintf(f, "Timestamp: %s\n", ts);
        fprintf(f, "Message: %s\n", message);
        if (folder != NULL) {
            fprintf(f, "Processed Folder: %s\n", folder);
            fprintf(f, "Total Files: %d\n", count);
        }
        fprintf(f, "--------------------------\n");
        fclose(f);
    }
}

int main(int argc, char *argv[]) {
    // Register signal handler for graceful shutdown
    signal(SIGINT, handle_sigint);

    if (argc < 2) {
        printf("Usage: ./monitor <source_directory>\n");
        return 1;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) { // Initialize anonymous pipe for IPC [cite: 103]
        perror("pipe");
        return 1;
    }

    pid_t pid = fork(); // Forking for separate Scanner (parent) and Logger (child) processes [cite: 103]

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) { // CHILD PROCESS: Dedicated Logger [cite: 10, 12]
        close(pipefd[1]); // Close unused write end
        FILE *report = fopen("logs/report.txt", "a");
        char buffer[2048]; // Buffer matching the expanded log message size

        /* Continuously listen to the pipe for logs from the parent process */
        while (read(pipefd[0], buffer, sizeof(buffer)) > 0) {
            char ts[32];
            get_timestamp(ts);
            fprintf(report, "[%s] %s\n", ts, buffer); // Log with time and date [cite: 88, 117]
            fflush(report);
        }

        fclose(report);
        close(pipefd[0]);
        exit(0);
    } else { // PARENT PROCESS: Scanner & Backup Controller [cite: 11-12]
        close(pipefd[0]); // Close unused read end

        int count = 0;
        int success_count = 0;
        FileInfo *files = scan_directory(argv[1], &count); // Dynamic memory allocation inside scanner [cite: 20, 109]

        if (count == 0) {
            /* Handle cases where the directory is empty or inaccessible */
            create_system_alert("EMPTY_SOURCE", "No .txt files found to process.", argv[1], 0);
            char empty_msg[512];
            sprintf(empty_msg, "WARNING: No files found in '%s'.", argv[1]);
            write(pipefd[1], empty_msg, strlen(empty_msg) + 1);
        } else if (files != NULL) {
            /* Iterate through found files and perform backups [cite: 115-116] */
            for (int i = 0; i < count; i++) {
                char log_msg[2048]; // Increased size to prevent overflow warnings
                char full_path[1024];
                
                /* Construct full relative path for logging accuracy */
                snprintf(full_path, sizeof(full_path), "%s/%s", argv[1], files[i].name);

                if (perform_backup(argv[1], files[i].name) == 0) { // Perform file copy [cite: 116]
                    snprintf(log_msg, sizeof(log_msg), "SUCCESS: File copied from path: %s", full_path);
                    success_count++;
                } else {
                    /* Log critical failure if a file cannot be backed up */
                    snprintf(log_msg, sizeof(log_msg), "CRITICAL: Failed to copy from path: %s", full_path);
                    create_system_alert("BACKUP_FAILURE", log_msg, argv[1], count);
                }
                
                /* Send path-aware log entry to child process via Pipe [cite: 116-117] */
                write(pipefd[1], log_msg, strlen(log_msg) + 1); 
            }
            
            /* Finalize session with a summary alert in the alerts/ directory */
            create_system_alert("FINAL_SUMMARY", "Backup cycle completed successfully.", argv[1], success_count);
            
            free(files); // Free allocated structures to prevent memory leaks 
        }

        close(pipefd[1]); // Close write end to signal child logger to terminate
        wait(NULL); // Reap child process to clean up system resources [cite: 124]
        printf("Process finished. Check logs/report.txt for details and alerts/ for summary.\n");
    }

    return 0;
}