#include "tester.h"
#include "%name%.h"

const char *TesterHelpStrings[] = {0};

TesterReturnValue TesterInitCallback()
{
    return TESTER_OK;
}

TesterReturnValue TesterParseCallback(const char *command)
{
    return TESTER_COUNT;
}

TesterReturnValue TesterExitCallback()
{
    return TESTER_OK;
}