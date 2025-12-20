#include "exec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>

static pid_t background_pids[100];
static int bg_count = 0;

static void add_background_pid(pid_t pid) {
    if (bg_count < 100) {
        background_pids[bg_count++] = pid;
    }
}

void execute_background_processes() {
    for (int i = 0; i < bg_count; i++) {
        if (background_pids[i] > 0) {
            int status;
            pid_t result = waitpid(background_pids[i], &status, WNOHANG);
            if (result > 0) {
                printf("[%d] Завершен процесс с PID %d\n", i+1, background_pids[i]);
                background_pids[i] = 0;
            }
        }
    }
}

int execute_cd(char **args) {
    if (args[1] == NULL) {
        char *home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, "cd: HOME не установлена\n");
            return 1;
        }
        if (chdir(home) != 0) {
            perror("cd");
            return 1;
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
            return 1;
        }
    }
    return 0;
}

int execute_pwd(char **args) {
    (void)args;
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        return 0;
    } else {
        perror("pwd");
        return 1;
    }
}

int execute_exit(char **args) {
    (void)args; 
    exit(0);
    return 0;
}

static int is_builtin(char *cmd) {
    if (cmd == NULL) return 0;
    return (strcmp(cmd, "cd") == 0 ||
            strcmp(cmd, "pwd") == 0 ||
            strcmp(cmd, "exit") == 0);
}

static int execute_builtin(tree t) {
    if (t->argv == NULL || t->argv[0] == NULL) return 0;
    
    char *cmd = t->argv[0];
    
    if (strcmp(cmd, "cd") == 0) {
        return execute_cd(t->argv);
    } else if (strcmp(cmd, "pwd") == 0) {
        return execute_pwd(t->argv);
    } else if (strcmp(cmd, "exit") == 0) {
        return execute_exit(t->argv);
    }
    
    return 0;
}

static int setup_redirections(tree t) {
    
    if (t->infile != NULL) {
        int fd = open(t->infile, O_RDONLY);
        if (fd < 0) {
            perror(t->infile);
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2 stdin");
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    if (t->outfile != NULL) {
        int flags = O_WRONLY | O_CREAT;
        flags |= t->append ? O_APPEND : O_TRUNC;
        
        int fd = open(t->outfile, flags, 0644);
        if (fd < 0) {
            perror(t->outfile);
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2 stdout");
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    return 0;
}


static int execute_pipeline(tree t) {
    int pipefd[2];
    int prev_pipe_read = -1;
    int status = 0;
    pid_t pid;
    
    while (t != NULL) {
        if (t->pipe != NULL) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                return -1;
            }
        }
        
        pid = fork();
        if (pid < 0) {
            perror("fork");
            return -1;
        }
        
        if (pid == 0) {
            if (setup_redirections(t) < 0) {
                exit(1);
            }
            
            if (prev_pipe_read != -1) {
                dup2(prev_pipe_read, STDIN_FILENO);
                close(prev_pipe_read);
            }
            
            if (t->pipe != NULL) {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
            
            if (prev_pipe_read != -1) close(prev_pipe_read);
            if (t->pipe != NULL) {
                close(pipefd[0]);
                close(pipefd[1]);
            }
            
            if (is_builtin(t->argv[0])) {
                int result = execute_builtin(t);
                exit(result);
            }
            
            execvp(t->argv[0], t->argv);
            perror(t->argv[0]);
            exit(1);
        } else {
            if (prev_pipe_read != -1) close(prev_pipe_read);
            if (t->pipe != NULL) {
                close(pipefd[1]);
                prev_pipe_read = pipefd[0];
            }
            
            if (t->backgrnd) {
                add_background_pid(pid);
            } else if (t->pipe == NULL) {
                waitpid(pid, &status, 0);
            }
        }
        
        t = t->pipe;
    }
    
    return status;
}

int execute_tree(tree t) {
    int status = 0;
    int last_status = 0;
    
    while (t != NULL) {
        if (t->backgrnd) {
            pid_t pid = fork();
            if (pid == 0) {
                int nullfd = open("/dev/null", O_RDONLY);
                if (nullfd >= 0) {
                    dup2(nullfd, STDIN_FILENO);
                    close(nullfd);
                }
                
                signal(SIGINT, SIG_IGN);
                
                if (t->pipe != NULL) {
                    execute_pipeline(t);
                } else {
                    if (setup_redirections(t) == 0) {
                        if (is_builtin(t->argv[0])) {
                            execute_builtin(t);
                        } else {
                            execvp(t->argv[0], t->argv);
                            perror(t->argv[0]);
                        }
                    }
                }
                exit(0);
            } else if (pid > 0) {
                add_background_pid(pid);
                printf("[%d] %d\n", bg_count, pid);
            }
            status = 0;
        } else {
            if (t->pipe != NULL) {
                status = execute_pipeline(t);
            } else {
                if (is_builtin(t->argv[0])) {
                    status = execute_builtin(t);
                } else {
                    pid_t pid = fork();
                    if (pid == 0) {
                        if (setup_redirections(t) < 0) {
                            exit(1);
                        }
                        execvp(t->argv[0], t->argv);
                        perror(t->argv[0]);
                        exit(1);
                    } else if (pid > 0) {
                        waitpid(pid, &status, 0);
                    }
                }
            }
            
            last_status = status;
            
            if (t->type == AND && status != 0) {
                if (t->next != NULL) {
                    t = t->next->next;
                    continue;
                }
            } else if (t->type == OR && status == 0) {
                if (t->next != NULL) {
                    t = t->next->next;
                    continue;
                }
            }
        }
        
        t = t->next;
    }
    
    return last_status;
}

void setup_signal_handlers() {
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
}