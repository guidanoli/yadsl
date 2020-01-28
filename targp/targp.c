#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "targp.h"

int targp(int argc, char **argv, const char **helpStrings, Command *cmds,
    void (*error_cb)(int argi, char *arg, char *errorMessage))
{
    /* argi points to the current argument */
    int argi = 1, cmd_argc;
    /* If no arguments are given, print help */
    if (argc == 1)
        for (; *helpStrings; ++helpStrings) puts(*helpStrings);
    /* Parse the rest of the arguments */
    while (argi < argc) {
        Command *pCmd, *currCmd = NULL;
        char *errorMessage;
        /* Search for a command that matches the current argument */
        for (pCmd = cmds; pCmd->cmd; ++pCmd) {
            if (!strcmp(pCmd->cmd, argv[argi])) {
                currCmd = pCmd;
                break;
            }
        }
        /* If could not find, throw error */
        if (currCmd == NULL) {
            fprintf(stderr, "[%s] \"%s\" is not a valid command.\n",
                argv[0], argv[argi]);
            return 1;
        }
        /* Execute command and move to the next one */
        cmd_argc = currCmd->argc;
        if (errorMessage = currCmd->parse_cb(cmd_argc, argv + argi + 1)) {
            error_cb(argi, argv[argi], errorMessage);
            return 1;
        }
        argi += cmd_argc + 1;
    }
    return 0;
}