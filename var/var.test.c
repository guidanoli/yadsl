#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "var.h"
#include "targp.h"

#define VARCNT 10
#define MAXARGCNT 10

DEFPARSECB(add) /* Add variable */
DEFPARSECB(cmp) /* Compare variables */
DEFPARSECB(dmp) /* Dump variable */
DEFPARSECB(expect) /* Expect error */
DEFPARSECB(r) /* Read */
DEFPARSECB(w) /* Write */

static Command cmds[] = {
    DEFCMD(add, 2),
    DEFCMD(cmp, 2),
    DEFCMD(dmp, 1),
    DEFCMD(expect, 0),
    DEFCMD(r, 2),
    DEFCMD(w, 2),
    END_CMDS
};

static const char *helpStrings[] = {
    "This is an interactive module of the variable library",
    "You can interact with multiple variable objects at a time",
    "Actions are parsed by command line arguments",
    "The registered actions are the following:",
    "",
    "add <index> <text>     adds variable from text to index",
    "cmp <index1> <index2>  compares variables from two indices",
    "dmp <index>            dump variable contents",
    "expect                 expect error"
    "r <index> <filename>   serialize (read) from file",
    "w <index> <filename>   serialize (write) variable to file",
    NULL,
};

/* Global fields */

static VarReturnID varId = VAR_RETURN_OK;
static Variable *vars[VARCNT];
static int expectError = 0;

/* Private functions prototypes */

static void initVarsArray();
static int houseKeep();
static int parseIndex(char *text, int *pIndex);
static void error_cb(int argi, char *arg, char *errorMessage);

/* Main function */

int main(int argc, char **argv)
{
    int ret;
    initVarsArray();
    ret = targp(argc, argv, helpStrings, cmds, error_cb);
    if (expectError) ret = !ret;
    if (houseKeep()) ret = 1;
    return ret;
}

static void initVarsArray()
{
    static Variable **pVars = vars;
    size_t i, size = sizeof(vars)/sizeof(*vars);
    for (i = 0; i < size; ++i) *(pVars++) = NULL;
}

static int houseKeep()
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
    if (varGetRefCount()) {
        puts("Memory leak detected");
        return 1;
    }
#endif
    return 0;
}

static void error_cb(int argi, char *arg, char *errorMessage)
{
    if (varId) {
        fprintf(stderr, "Error #%d on argument #%d '%s': %s\n",
            varId, argi, arg, errorMessage);
    } else {
        fprintf(stderr, "Error on argument #%d '%s': %s\n",
            argi, arg, errorMessage);
    }
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

static char *_expect(int argc, char **argv)
{
    expectError = 1;
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