#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s f\n", argv[0]);
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // Дочерний процесс
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            perror("open");
            exit(1);
        }

        // Перенаправляем стандартный ввод из файла
        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }
        close(fd);

        // Выполняем команду cat
        execlp("cat", "cat", NULL);
        perror("execlp");
        exit(1);
    } else {
        // Родительский процесс
        wait(NULL);
    }

    return 0;
}