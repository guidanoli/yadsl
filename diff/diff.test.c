#include <stdio.h>
#include <string.h>
#include "diff.h"
#include "tester.h"

const char *TesterHelpStrings[] = {
    "/diff X Y    saves the difference between X and Y in slot A",
    "/equ B       asserts last value computed is equal to B",
    "/neq B       asserts last value computed is different from B",
    "/gtr B       asserts last value computed is greater than B",
    "/lwr B       asserts last value computed is lower than B",
    "/geq B       asserts last value computed is greater or equal to B",
    "/leq B       asserts last value computed is lower or equal to B",
    NULL,
};

#define matches(a, b) (strcmp(a, b) == 0)

char X[TESTER_BUFFER_SIZE], Y[TESTER_BUFFER_SIZE];
size_t val, eax = -1; /* Last computed value */

TesterReturnValue TesterInitCallback()
{
    return TESTER_RETURN_OK;
}

TesterReturnValue TesterParseCallback(const char *command)
{
    if matches(command, "diff") {
        if (TesterParseArguments("ss", X, Y) != 2)
            return TESTER_RETURN_ARGUMENT;
        eax = diff(X, Y);
        TesterLog("diff = %lu", eax);
    } else if matches(command, "equ") {
        if (TesterParseArguments("i", &val) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (!(eax == val))
            return TESTER_RETURN_RETURN;
    } else if matches(command, "neq") {
        if (TesterParseArguments("i", &val) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (!(eax != val))
            return TESTER_RETURN_RETURN;
    } else if matches(command, "gtr") {
        if (TesterParseArguments("i", &val) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (!(eax > val))
            return TESTER_RETURN_RETURN;
    } else if matches(command, "lwr") {
        if (TesterParseArguments("i", &val) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (!(eax < val))
            return TESTER_RETURN_RETURN;
    } else if matches(command, "geq") {
        if (TesterParseArguments("i", &val) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (!(eax >= val))
            return TESTER_RETURN_RETURN;
    } else if matches(command, "leq") {
        if (TesterParseArguments("i", &val) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (!(eax <= val))
            return TESTER_RETURN_RETURN;
    } else {
        return TESTER_RETURN_COMMAND;
    }
    return TESTER_RETURN_OK;
}

TesterReturnValue TesterExitCallback()
{
    return TESTER_RETURN_OK;
}