#include "mysh.h"

int main() {
    char input[MAX_CMD_LENGTH], cmd_copy[MAX_CMD_LENGTH];   // cmd_copy will be used in cmd_history_check as a string dummy for strtok
    char *cmd_args[MAX_ARGS];
    int num_args;
    myHistory cmd_history;
    cmd_history.newest_cmd = -1;
    cmd_history.oldest_cmd = 0;
    cmd_history.size = 0;

    while(1) {
        printf("in-mysh-now:> ");
        fflush(stdout);
        if(fgets(input, MAX_CMD_LENGTH, stdin) == NULL)
            break;

        if(input[0] == '\0')
            continue;

        save_cmd(&cmd_history, input);

        num_args = parse_input(input, cmd_args);

        if(!strcmp(cmd_args[0], "exit"))
            break;

        if(cmd_history_check(&cmd_history, cmd_args, &num_args, cmd_copy))
            continue;
        
        execute_cmd(cmd_args, num_args);
    }

    return 0;
}