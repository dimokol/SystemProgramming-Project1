#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>

#define MAX_CMD_LENGTH 64
#define MAX_ARGS 32
#define MAX_CMD_HISTORY 20

// myHistory structure to save command history 
typedef struct {
    char cmds[MAX_CMD_HISTORY][MAX_CMD_LENGTH];
    int newest_cmd;     // position of the newest command
    int oldest_cmd;     // position of the oldest command
    int size;           // number of commands saved
} myHistory;

// parse user's input into separete words (arguments)
int parse_input(char *input, char **cmd_args);

// save command to command history
void save_cmd(myHistory *cmd_h, char *cmd);

// check for command history commands and execute them
bool cmd_history_check(myHistory *cmd_history, char **cmd_args, int *num_args, char *cmd_copy);

// check for I/O redirection and complete it
void io_redirection(char **cmd_args, int *num_args);

// check for piping and complete it
void piping(char **cmd_args, int num_args);

// execute command
void execute_cmd(char **cmd_args, int num_args);