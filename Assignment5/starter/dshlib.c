#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"
#include <errno.h>

/*
 * Implement your exec_local_cmd_loop function by building a loop    that prompts the 
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
int exec_local_cmd_loop()
{
    char *cmd_buff = malloc(SH_CMD_MAX);
    int rc = 0;
    cmd_buff_t cmd;

    if (!cmd_buff) {
        return ERR_MEMORY;
    }

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        // Removing the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // If no command is given, continue
        if (strlen(cmd_buff) == 0) {
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        }

        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            free(cmd_buff);
            exit(OK);
        }

        // Handle cd command
        if (strncmp(cmd_buff, "cd", 2) == 0 && (cmd_buff[2] == '\0' || cmd_buff[2] == ' ')) {
            strtok(cmd_buff, " ");
            char *dir_arg = strtok(NULL, " ");

            if (dir_arg == NULL) {
                continue;
            } else {
                char *extra_arg = strtok(NULL, " ");
                if (extra_arg != NULL) {
                    printf("cd: error too many arguments!");
                    rc = ERR_TOO_MANY_COMMANDS;
                    continue;
                }

                if (chdir(dir_arg) != 0) {
                    printf(CMD_ERR_EXECUTE);
                    rc = ERR_EXEC_CMD;
                    continue;
                }
            }
            continue;
        }

        rc = build_cmd_buff(cmd_buff, &cmd);
        if (rc == OK) {
            rc = exec_cmd(&cmd);
        } else if (rc == WARN_NO_CMDS) {
            printf("%s", CMD_WARN_NO_CMD);
        } else {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
        }
    }

    free(cmd_buff);
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
{
    memset(cmd_buff, 0, sizeof(cmd_buff_t));

    cmd_buff->_cmd_buffer = strdup(cmd_line);
    if (!cmd_buff->_cmd_buffer) {
        return ERR_MEMORY;
    }

    bool inside_quotes = false;
    int arg_count = 0;

    while (*cmd_line) {
        while (*cmd_line == SPACE_CHAR && !inside_quotes) {
            cmd_line++;
        }

        if (*cmd_line == '\0') {
            if (arg_count > 0) {
                break;
            } else {
                free(cmd_buff->_cmd_buffer);
                return WARN_NO_CMDS;
            }
        }

        if (*cmd_line == QUOTES) {
            inside_quotes = !inside_quotes;
            cmd_line++;
        }

        cmd_buff->argv[arg_count] = cmd_line;
        arg_count++;

        if (arg_count > 0 && (strlen(cmd_buff->argv[arg_count - 1]) >= ARG_MAX || arg_count >= CMD_ARGV_MAX)) {
            free(cmd_buff->_cmd_buffer);
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        while (*cmd_line && (*cmd_line != SPACE_CHAR || inside_quotes)) {
            if (*cmd_line == QUOTES) {
                inside_quotes = !inside_quotes;
                *cmd_line = '\0';
            }
            cmd_line++;
        }

        if (*cmd_line == SPACE_CHAR) {
            *cmd_line = '\0';
            cmd_line++;
        }
    }

    cmd_buff->argc = arg_count;
    cmd_buff->argv[arg_count] = NULL;

    return OK;
}

int exec_cmd(cmd_buff_t *cmd)
{
    int pid = fork();

    if (pid < 0) {
        printf("fork failed\n");
        return ERR_MEMORY;
    } else if (pid == 0) {
        execvp(cmd->argv[0], cmd->argv);
        printf("command not found: %s\n", cmd->argv[0]);
        exit(ERR_EXEC_CMD);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

int last_return_code = 0;

