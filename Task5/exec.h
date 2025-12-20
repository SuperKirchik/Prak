#ifndef EXEC_H
#define EXEC_H

#include "tree.h"

int execute_tree(tree t);
void execute_background_processes();
void setup_signal_handlers();
int execute_cd(char **args);
int execute_pwd(char **args);
int execute_exit(char **args);

#endif /* EXEC_H */