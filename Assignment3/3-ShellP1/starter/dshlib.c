#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    // Clear out the command_list_t structure.
    memset(clist, 0, sizeof(command_list_t));
    char *cmd_segment = strtok(cmd_line, PIPE_STRING);

    while (cmd_segment != NULL){
        if (clist->num >= CMD_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }

        while (isspace((unsigned char)*cmd_segment)){
            cmd_segment++;
        }

        
        char *arg_ptr = strpbrk(cmd_segment, " \t");
        if (arg_ptr != NULL)
        {
            // Terminate the executable name string.
            *arg_ptr = '\0';
            arg_ptr++;

            // Skip any additional whitespace before actual arguments.
            while (isspace((unsigned char)*arg_ptr)){
                arg_ptr++;
            }

            int arg_len = strlen(arg_ptr);
            while (arg_len > 0 && isspace((unsigned char)arg_ptr[arg_len - 1])){
                arg_ptr[arg_len - 1] = '\0';
                arg_len--;
            }

            if (strlen(arg_ptr) >= ARG_MAX)
            {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            // Store the arguments in the command list.
            strcpy(clist->commands[clist->num].args, arg_ptr);
        }
        else{
            clist->commands[clist->num].args[0] = '\0';
        }
        if (strlen(cmd_segment) >= EXE_MAX)
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;}

        strcpy(clist->commands[clist->num].exe, cmd_segment);

        clist->num++;

        cmd_segment = strtok(NULL, PIPE_STRING);
    }

    return OK;
}