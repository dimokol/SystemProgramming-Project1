#include "mysh.h"

int parse_input(char *input, char **cmd_args) {
    int num_args = 0;
    char *arg = strtok(input, " \t\n");
    while(arg != NULL && num_args <= MAX_ARGS) {
        cmd_args[num_args] = arg;
        num_args++;
        arg = strtok(NULL, " \t\n");
    }
    cmd_args[num_args] = NULL;
    return num_args;
}

// g) Support command history
void save_cmd(myHistory *cmd_h, char *cmd) {
    cmd_h->newest_cmd = (cmd_h->newest_cmd + 1) % MAX_CMD_HISTORY;  // update the position this command will be saved to
    strcpy(cmd_h->cmds[cmd_h->newest_cmd], cmd);    // save command (if history is full, the oldest command will be overwritten)
    if(cmd_h->size < MAX_CMD_HISTORY)   // if history is not full yet
        cmd_h->size++;                  // increase size
    else                                // else keep track of the oldest command to print as first
        cmd_h->oldest_cmd = (cmd_h->oldest_cmd + 1) % MAX_CMD_HISTORY;
}

bool cmd_history_check(myHistory *cmd_history, char **cmd_args, int *num_args, char *cmd_copy) {
    if(!strcmp(cmd_args[0], "myHistory")) {
            if(*num_args == 1) {    // if no number of commands is specified print them all
                for(int i = 0; i < cmd_history->size; i++)
                    printf("    %d  %s", i+1, cmd_history->cmds[(cmd_history->oldest_cmd + i) % MAX_CMD_HISTORY]);  // in the order they were given
                
                return true;
            }
            else {
                int cmd_num = atoi(cmd_args[1]);
                if(cmd_num > 0 && cmd_num <= cmd_history->size) {   // if it's a valid number print and send the command to be executed
                    printf("    %d  %s", cmd_num, cmd_history->cmds[cmd_num-1]);
                    for(int i = 0; i < *num_args; i++)  // empty cmd_args to replace with the command from history
                        memset(cmd_args[i],0,strlen(cmd_args[i]));

                    strcpy(cmd_copy, cmd_history->cmds[cmd_num-1]); // send a copy so command from history doesn't get messed from strtok
                    *num_args = parse_input(cmd_copy, cmd_args);
                    return cmd_history_check(cmd_history, cmd_args, num_args, cmd_copy);
                }
                else {
                    printf("myHistory %s: event not found\n", cmd_args[1]);
                    return true;
                }
            }
    }
    return false;
}

// a) Support I/O redirection(</>) and addition to existing file (>>)
void io_redirection(char **cmd_args, int *num_args) {
    // output redirection
    for(int i = 0; i < *num_args - 1; i++) {
        if(!strcmp(cmd_args[i], ">")) {
            int output_file = open(cmd_args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0777); // O_WRONLY: write only, O_CREAT: create file if doesn't exist, O_TRUNC: truncate (replace) file, 0777: everybody has access (<fcntl.h>)
            if(output_file == -1) {
                perror("open");
                exit(1);
            }
            
            if(dup2(output_file, STDOUT_FILENO) == -1) { // redirect stdout to output_file
                perror("dup2");
                exit(1);
            }
            if(dup2(output_file, STDERR_FILENO) == -1) { // redirect stderr to output_file
                perror("dup2");
                exit(1);
            }

            close(output_file);
            cmd_args[i] = NULL; // remove >
            cmd_args[i+1] = NULL; // and filename from args
            *num_args -= 2;
            break;
        }
        if(!strcmp(cmd_args[i], ">>")) {
            int output_file = open(cmd_args[i+1], O_WRONLY | O_CREAT | O_APPEND, 0777); // “.., O_APPEND: append to file, ..“
            if(output_file == -1) {
                perror("open");
                exit(1);
            }
            
            if(dup2(output_file, STDOUT_FILENO) == -1) { // redirect stdout to output_file
                perror("dup2");
                exit(1);
            }
            if(dup2(output_file, STDERR_FILENO) == -1) { // redirect stderr to output_file
                perror("dup2");
                exit(1);
            }

            close(output_file);
            cmd_args[i] = NULL; // remove >
            cmd_args[i+1] = NULL; // and filename from args
            *num_args -= 2;
            break;
        }
    }

    // input redirection
    for(int i = 0; i < *num_args - 1; i++) {
        if(!strcmp(cmd_args[i], "<")) {
            int input_file = open(cmd_args[i+1], O_RDONLY); // O_WRONLY: read only
            if(input_file == -1) {
                perror("open");
                exit(1);
            }
            
            if(dup2(input_file, STDIN_FILENO) == -1) { // redirect stdin to input_file
                perror("dup2");
                exit(1);
            }

            close(input_file);
            cmd_args[i] = NULL; // remove <
            cmd_args[i+1] = NULL; // and filename from args
            *num_args -= 2;
            break;
        }
    }
}

// b) Support pipes (|)
void piping(char **cmd_args, int num_args) {
    for(int i = 0; i < num_args - 1; i++) {
        if(!strcmp(cmd_args[i], "|")) {
            cmd_args[i] = NULL; // remove | to not interfere with the 2 separate commands
            char **cmd2_args = &cmd_args[i+1]; // command after |
            int num_args2 = num_args - i - 1; // and number of it's arguments
            int num_args1 = i; // number of arguments of the command before |

            int pipefd[2];
            if(pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }

            pid_t pid2 = fork();
            if(pid2 < 0) {
                perror("fork");
                exit(1);
            }
            else if(pid2 == 0) {    // child process will deal with the left side of the pipe
                // child process 2

                close(pipefd[0]); // close read end of pipe
                if(dup2(pipefd[1], STDOUT_FILENO) == -1) { // redirect stdout to write end of pipe
                    perror("dup2");
                    exit(1);
                }
                // if(dup2(pipefd[1], STDERR_FILENO) == -1) { // redirect stderr to write end of pipe
                //     perror("dup2");
                //     exit(1);
                // }
                close(pipefd[1]); // close write end of pipe

                // io redirection
                io_redirection(cmd_args, &num_args1);

                // command execution
                execvp(cmd_args[0], cmd_args);
                perror("execvp");   // if execvp returns here it means it failed
                // printf("%s: command not found.\n", cmd_args[0]);
                exit(1);
            }
            else {  // parent process will deal with the right side of the pipe
                // parent process (child process)

                close(pipefd[1]); // close write end of pipe
                if(dup2(pipefd[0], STDIN_FILENO) == -1) { // redirect stdin to read end of pipe
                    perror("dup2");
                    exit(1);
                }
                close(pipefd[0]); // close read end of pipe

                // io redirection
                io_redirection(cmd2_args, &num_args2);

                // command execution
                execvp(cmd2_args[0], cmd2_args);
                perror("execvp");   // if execvp returns here it means it failed
                // printf("%s: command not found.\n", cmd2_args[0]);
                exit(1);
            }
        }
    }
}

void execute_cmd(char **cmd_args, int num_args) {
    int status;
    pid_t pid = fork();
    if(pid < 0) {
        perror("fork");
        exit(1);
    }
    else if(pid == 0) {
        // child process

        // pipes
        piping(cmd_args, num_args);

        // io redirection
        io_redirection(cmd_args, &num_args);

        // command execution
        execvp(cmd_args[0], cmd_args); 
        perror("execvp");   // if execvp returns here it means it failed
        // printf("%s: command not found.\n", cmd_args[0]);
        exit(1);
    }
    else {
        // parent process

        waitpid(pid, &status, 0);
    }
}