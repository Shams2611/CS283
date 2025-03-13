#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
 int exec_local_cmd_loop() {
    char *cmd_buff = malloc(SH_CMD_MAX);
    if (!cmd_buff) {
        return ERR_MEMORY;
    }

    command_list_t clist;

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        if (strlen(cmd_buff) == 0) {
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        }
        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            free(cmd_buff);
            printf("exiting...\n");
            return OK;
        }

        if (strncmp(cmd_buff, "cd", 2) == 0 && (cmd_buff[2] == '\0' || cmd_buff[2] == ' ')) {
            char *path = strtok(cmd_buff + 3, " ");
            if (path == NULL) {
                continue; 
            }

            if (chdir(path) != 0) {
                printf("error: command failed\n"); 
            }
            continue;
        }
        int rc = build_cmd_list(cmd_buff, &clist);
        if (rc != OK) {
            if (rc == ERR_TOO_MANY_COMMANDS) {
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            } else if (rc == ERR_CMD_OR_ARGS_TOO_BIG) {
                printf("error: command or arguments too big\n");
            } else if (rc == WARN_NO_CMDS) {
                printf("%s", CMD_WARN_NO_CMD);
            }
            continue;
        }
        execute_pipeline(&clist);
    }

    free(cmd_buff);
    return OK;
}

int build_cmd_list(char *cmd_line, command_list_t *clist){
    memset(clist, 0, sizeof(command_list_t));
    char *token = strtok(cmd_line, PIPE_STRING);

    while (token != NULL) {
        if (clist->num >= CMD_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }
        while (*token == SPACE_CHAR) {
            token++;
        }
        int token_len = strlen(token);
        while (token_len > 0 && token[token_len - 1] == SPACE_CHAR){
            token[token_len - 1] = '\0';
            token_len--;
        }

        if (strlen(token) >= EXE_MAX) 
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        int rc = build_cmd_buff(token, &clist->commands[clist->num]);
        if (rc != OK) {
            return rc;
        }

        clist->num++;
        token = strtok(NULL, PIPE_STRING);
    }


    return OK;
}

int build_cmd_buff(char *cmd, cmd_buff_t *cbuff) {
    memset(cbuff, 0, sizeof(cmd_buff_t));
    cbuff->_cmd_buffer = strdup(cmd);
    if (!cbuff->_cmd_buffer) {
        return ERR_MEMORY;
    }

    bool in_quotes = false;
    int arg_count = 0;

    // Initialize redirection fields
    cbuff->input_file = NULL;
    cbuff->output_file = NULL;
    cbuff->append_output = false;

    while (*cmd) {
        while (*cmd == SPACE_CHAR && !in_quotes) {
            cmd++;
        }

        if (*cmd == '\0') {
            if (arg_count == 0) {
                free(cbuff->_cmd_buffer);
                return WARN_NO_CMDS;
            }
            break;
        }

        // Handle input redirection
        if (*cmd == '<' && !in_quotes) {
            *cmd = '\0'; // Terminate the command
            cmd++;
            while (*cmd == SPACE_CHAR) cmd++; // Skip spaces
            cbuff->input_file = cmd;
            while (*cmd && *cmd != SPACE_CHAR) cmd++; // Move to end of filename
            *cmd = '\0'; // Terminate the filename
            cmd++;
            continue;
        }

        // Handle output redirection
        if (*cmd == '>' && !in_quotes) {
            *cmd = '\0'; // Terminate the command
            cmd++;
            if (*cmd == '>') { // Handle append (>>)
                cbuff->append_output = true;
                cmd++;
            }
            while (*cmd == SPACE_CHAR) cmd++; // Skip spaces
            cbuff->output_file = cmd;
            while (*cmd && *cmd != SPACE_CHAR) cmd++; // Move to end of filename
            *cmd = '\0'; // Terminate the filename
            cmd++;
            continue;
        }

        if (*cmd == '"') {
            in_quotes = !in_quotes;
            cmd++;
        }

        cbuff->argv[arg_count] = cmd;
        arg_count++;

        if (arg_count >= CMD_ARGV_MAX || strlen(cbuff->argv[arg_count - 1]) >= ARG_MAX) {
            free(cbuff->_cmd_buffer);
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        while (*cmd && (*cmd != SPACE_CHAR || in_quotes)) {
            if (*cmd == '"') {
                in_quotes = !in_quotes;
                *cmd = '\0';
            }
            cmd++;
        }

        if (*cmd == SPACE_CHAR) {
            *cmd = '\0';
            cmd++;
        }
    }

    cbuff->argc = arg_count;
    cbuff->argv[arg_count] = NULL;

    return OK;
}
int execute_pipeline(command_list_t *clist) {
    int pipe_fds[2 * (clist->num - 1)];
    pid_t pids[clist->num];
    int prev_pipe_read = -1;

    for (int i = 0; i < clist->num; i++) {
        if (i < clist->num - 1) {
            if (pipe(pipe_fds + i * 2) < 0) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pids[i] == 0) {
            // Handle input redirection
            if (clist->commands[i].input_file) {
                int fd = open(clist->commands[i].input_file, O_RDONLY);
                if (fd < 0) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // Handle output redirection
            if (clist->commands[i].output_file) {
                int flags = O_WRONLY | O_CREAT;
                if (clist->commands[i].append_output) {
                    flags |= O_APPEND; // Append mode for >>
                } else {
                    flags |= O_TRUNC; // Overwrite mode for >
                }
                int fd = open(clist->commands[i].output_file, flags, 0644);
                if (fd < 0) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if (i > 0) {
                dup2(prev_pipe_read, STDIN_FILENO);
                close(prev_pipe_read);
            }

            if (i < clist->num - 1) {
                dup2(pipe_fds[i * 2 + 1], STDOUT_FILENO);
                close(pipe_fds[i * 2]);
                close(pipe_fds[i * 2 + 1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            if (i > 0) {
                close(prev_pipe_read);
            }
            if (i < clist->num - 1) {
                prev_pipe_read = pipe_fds[i * 2];
                close(pipe_fds[i * 2 + 1]);
            }
        }
    }

    for (int i = 0; i < clist->num; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }

    return OK;
}

int free_cmd_list(command_list_t *cmd_lst) {
    if (!cmd_lst) {
        return ERR_MEMORY;
    }

    for (int i = 0; i < cmd_lst->num; i++) {
        free(cmd_lst->commands[i]._cmd_buffer);
    }

    return OK;
}