#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "var.h"

#define VARCNT 10
#define MAXARGCNT 10

typedef struct
{
    const char *cmd;
    char *argTypes;
    char *(*parse_cb)(int argc, char **argv);
}
Command;

/* Command arguments */

#define DEFPARSECB(label) static char * _ ## label (int argc, char **argv);
#define DEFCMD(label, cmds) { #label , cmds, _ ## label },

DEFPARSECB(add) /* Add variable */
DEFPARSECB(cmp) /* Compare variables */
DEFPARSECB(dmp) /* Dump variable */
DEFPARSECB(help) /* Help */
DEFPARSECB(r) /* Read */
DEFPARSECB(w) /* Write */

static Command cmds[] = {
    DEFCMD(add, "it")
    DEFCMD(cmp, "ii")
    DEFCMD(dmp, "i")
    DEFCMD(help, "")
    DEFCMD(r, "it")
    DEFCMD(w, "it")
    { NULL, NULL, NULL }, /* Sentinel -- don't mess with it */
};

/* Help */

static const char *helpStrings[] = {
    "This is an interactive module of the variable library",
    "You can interact with multiple variable objects at a time",
    "Actions are parsed by command line arguments",
    "The registered actions are the following:",
    "",
    "add <index> <text>     adds variable from text to index",
    "cmp <index1> <index2>  compares variables from two indices",
    "dmp <index>            dump variable contents",
    "help                   get further help",
    "r <index> <filename>   serialize (read) from file",
    "w <index> <filename>   serialize (write) variable to file",
    NULL, /* Sentinel -- don't mess with it */
};

/* Global fields */

static VarReturnID varId = VAR_RETURN_OK;
static Variable *vars[VARCNT];

/* Private functions prototypes */

static void initVarsArray();
static void houseKeep();
static int parseIndex(char *text, int *pIndex);

/* Main function */

int main(int argc, char **argv)
{
    /* argi points to the current argument */
    int argi = 0, cmd_argc;
    initVarsArray();
    /* Disconsider program name */
    --argc;
    ++argv;
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
            fprintf(stderr, "\"%s\" is not a valid command.\n", argv[argi]);
            houseKeep();
            return 1;
        }
        /* Execute command and move to the next one */
        cmd_argc = strlen(currCmd->argTypes);
        if (errorMessage = currCmd->parse_cb(cmd_argc, argv + argi + 1)) {
            if (varId) {
                fprintf(stderr, "Error on command #%d (\"%s\"): %s\n",
                    argi + 1, argv[argi], errorMessage);
            } else {
                fprintf(stderr, "Error #%d on command #%d (\"%s\"): %s\n",
                    varId, argi + 1, argv[argi], errorMessage);
            }
            houseKeep();
            return 1;
        }
        argi += cmd_argc + 1;
    }
    houseKeep();
    return 0;
}

static void initVarsArray()
{
    static Variable **pVars = vars;
    size_t i, size = sizeof(vars)/sizeof(*vars);
    for (i = 0; i < size; ++i) *(pVars++) = NULL;
}

static void houseKeep()
{
    static Variable **pVars = vars;
    size_t i, size = sizeof(vars)/sizeof(*vars);
    for (i = 0; i < size; ++i) {
        if (pVars[i] != NULL) {
            varDestroy(pVars[i]);
            pVars[i] = NULL;
        }
    }
#ifdef _DEBUG
    if (varGetRefCount())
        puts("Memory leak detected");
#endif
}

static char *_add(int argc, char **argv)
{
    int index;
    if (parseIndex(argv[0], &index))
        return "Could not parse index";
    if (vars[index]) {
        varDestroy(vars[index]);
        vars[index] = NULL;
    }
    if (varId = varCreate(argv[1], &vars[index]))
        return "Could not create variable";
    return NULL;
}

static char *_cmp(int argc, char **argv)
{
    int indexA, indexB, result;
    if (parseIndex(argv[0], &indexA))
        return "Could not parse first index";
    if (parseIndex(argv[1], &indexB))
        return "Could not parse second index";
    if (!vars[indexA] || !vars[indexB])
        return "One of the two variables does not exist";
    if (varId = varCompare(vars[indexA], vars[indexB], &result))
        return "Could not compare variables";
    printf("Variables are %s\n", result ? "equal" : "different");
    return NULL;
}

static char *_dmp(int argc, char **argv)
{
    int index;
    if (parseIndex(argv[0], &index))
        return "Could not parse index";
    if (!vars[index])
        return "Variable does not exist";
    if (varId = varWrite(vars[index], stdout))
        return "Could not dump variable";
    printf("\n");
    return NULL;
}

static char *_help(int argc, char **argv)
{
    const char **str = helpStrings;
    for (; *str; ++str) puts(*str);
    return NULL;
}

static char *_r(int argc, char **argv)
{
    int index;
    FILE *f;
    if (parseIndex(argv[0], &index))
        return "Could not parse index";
    if (vars[index]) {
        varDestroy(vars[index]);
        vars[index] = NULL;
    }
    if ((f = fopen(argv[1], "r")) == NULL)
        return "Could not open file in reading mode";
    varId = varSerializeRead(&vars[index], f);
    fclose(f);
    if (varId)
        return "Could not read from file to variable";
    return NULL;
}

static char *_w(int argc, char **argv)
{
    int index;
    FILE *f;
    if (parseIndex(argv[0], &index))
        return "Could not parse index";
    if (!vars[index])
        return "Variable does not exist";
    if ((f = fopen(argv[1], "w")) == NULL)
        return "Could not open file in writing mode";
    varId = varSerializeWrite(vars[index], f);
    fclose(f);
    if (varId)
        return "Could not write variable to file";
    return NULL;
}

static int parseIndex(char *text, int *pIndex)
{
    int index;
    char end = 0;
    if (text == NULL || pIndex == NULL)
        return 1; // invalid parameters
    if (sscanf(text, "%d%c", &index, &end) != 1 || end)
        return 1; // string isn't numeric
    if (index < 0 || index > VARCNT)
        return 1; // index is invalid
    *pIndex = index;
    return 0;
}