#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "exec.h"

int shell_should_exit = 0;
int shell_exit_code = 0;

static int handle_builtin(tree *node) {
    if (!node || !node->argv || !node->argv[0]) return 0;
    
    char *cmd = node->argv[0];
    
    // cd
    if (strcmp(cmd, "cd") == 0) {
        if (node->argv[1]) {
            if (chdir(node->argv[1]) != 0) {
                perror("cd");
                return 1;
            }
        } else {
            char *home = getenv("HOME");
            if (home) chdir(home);
        }
        return 1; 
    }
    
    // exit
     if (strcmp(cmd, "exit") == 0) {
        if (node->argv[1]) {
            shell_exit_code = atoi(node->argv[1]);
        }
        shell_should_exit = 1;
        return 1;
    }
    
    // pwd
    if (strcmp(cmd, "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd");
        }
        return 1;
    }
    
    // echo
    if (strcmp(cmd, "echo") == 0) {
        for (int i = 1; node->argv[i]; i++) {
            printf("%s%s", node->argv[i], node->argv[i+1] ? " " : "");
        }
        printf("\n");
        return 1;
    }
    
    return 0;  
}

void setup_redir(tree *node) {
    if (node->infile) {
        int fd = open(node->infile, O_RDONLY);
        if (fd < 0) {
            perror("open");
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    
    if (node->outfile) {
        int flags = O_WRONLY | O_CREAT;
        if (node->append) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }
        
        int fd = open(node->outfile, flags, 0644);
        if (fd < 0) {
            perror("open");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

int exec_single(tree *node) {
    if (!node || !node->argv[0]) return 0;
    
    // Проверяем внутренние команды
    if (handle_builtin(node)) {
        return 0;
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        // Дочерний
        if (node->background) {
            signal(SIGINT, SIG_IGN);
            int null_fd = open("/dev/null", O_RDONLY);
            dup2(null_fd, STDIN_FILENO);
            close(null_fd);
        }
        
        // Перенаправления
        if (node->infile) {
            int fd = open(node->infile, O_RDONLY);
            if (fd < 0) {
                perror("open");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        
        if (node->outfile) {
            int flags = O_WRONLY | O_CREAT;
            if (node->append) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            
            int fd = open(node->outfile, flags, 0644);
            if (fd < 0) {
                perror("open");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        
        execvp(node->argv[0], node->argv);
        perror("execvp");
        exit(1);
    }
    
    int status;
    
    if (node->background) {
        printf("[%d] running in background\n", pid);
        return 0;
    } else {
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

int exec_pipe(tree *node) {

    tree *check = node;
    while (check) {
        if (check->argv && check->argv[0]) {
            char *cmd = check->argv[0];
            // Команды, которые не должны быть в пайпах
            if (strcmp(cmd, "cd") == 0) {
                fprintf(stderr, "Error: cd cannot be used in a pipeline\n");
                return 1;
            }
            if (strcmp(cmd, "exit") == 0) {
                fprintf(stderr, "Error: exit cannot be used in a pipeline\n");
                return 1;
            }
            // echo и pwd могут быть в пайпах
        }
        check = check->pipe;
    }

    int input_fd = -1;
    pid_t last_pid = -1;
    tree *current = node;
    
    while (current) {

        int pipefd[2];
        
        if (current->pipe) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                return -1;
            }
        }
        
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork");
            return -1;
        }
        
        if (pid == 0) {
            if (input_fd != -1) {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
            
            if (current->pipe) {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
            
            setup_redir(current);
            execvp(current->argv[0], current->argv);
            perror("execvp");
            exit(1);
        }
        
        if (input_fd != -1) {
            close(input_fd);
        }
        
        if (current->pipe) {
            close(pipefd[1]);
            input_fd = pipefd[0];
        }
        
        last_pid = pid;
        current = current->pipe;
    }
    
    int status;
    if (last_pid != -1) {
        waitpid(last_pid, &status, 0);
        return WEXITSTATUS(status);
    }
    
    return 0;
}

int exec_tree(tree *node) {
    if (!node) return 0;
    
    int status = 0;
    
    if (node->pipe) {
        status = exec_pipe(node);
    } else {
        status = exec_single(node);
    }
    
    if (node->next) {
        if (node->type == AND) {
            if (status == 0) {
                exec_tree(node->next);
            }
        } else if (node->type == OR) {
            if (status != 0) {
                exec_tree(node->next);
            }
        } else {
            exec_tree(node->next);
        }
    }
    
    return status;
}