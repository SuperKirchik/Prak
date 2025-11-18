#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s cmd f\n", argv[0]);
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // Дочерний процесс
        int fd = open(argv[2], O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd == -1) {
            perror("open");
            exit(1);
        }

        // Перенаправляем стандартный вывод в файл
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }
        close(fd);

        // Выполняем команду
        execlp(argv[1], argv[1], NULL);
        perror("execlp");
        exit(1);
    } else {
        // Родительский процесс
        wait(NULL);
    }

    return 0;
}