#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "tree.h"

int exec_tree(tree *node);
extern int shell_should_exit;
extern int shell_exit_code;

#endif