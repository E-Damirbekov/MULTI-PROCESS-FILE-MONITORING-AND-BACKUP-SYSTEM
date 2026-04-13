#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include "scanner.h"
#include "backup.h"

void handle_sigint(int sig) {
    printf("\n[SIGINT] Завершение работы системы...\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);

    if (argc < 2) {
        printf("Использование: ./monitor <папка_источник>\n");
        return 1;
    }

    int pipefd[2];
    pipe(pipefd); // Создаем Pipe для передачи логов

    pid_t pid = fork(); // Создаем дочерний процесс

    if (pid == 0) { // Дочерний процесс (LOGGER)
        close(pipefd[1]);
        FILE *log_file = fopen("logs/report.txt", "a");
        char msg[512];
        while (read(pipefd[0], msg, sizeof(msg)) > 0) {
            fprintf(log_file, "[CHILD LOG]: %s\n", msg);
            fflush(log_file);
        }
        fclose(log_file);
        close(pipefd[0]);
        exit(0);
    } else { // Родительский процесс (SCANNER)
        close(pipefd[0]);
        int count = 0;
        FileInfo *files = scan_directory(argv[1], &count);

        if (files) {
            for (int i = 0; i < count; i++) {
                perform_backup(argv[1], files[i].name);
                char log_msg[512];
                sprintf(log_msg, "Файл %s сохранен", files[i].name);
                write(pipefd[1], log_msg, strlen(log_msg) + 1);
            }
            free(files);
        }
        close(pipefd[1]);
        wait(NULL); // Ждем завершения логгера
    }
    return 0;
}