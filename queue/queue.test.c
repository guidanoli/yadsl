#include "queue.h"

#include <stdio.h>
#include <string.h>

#include "tester.h"
#include "var.h"

#define matches(a, b) (strcmp(a, b) == 0)

const char *TesterHelpStrings[] = {
    "This is the queue test module",
    "You will be interacting with the same queue at all times",
    "The available actions are the following:",
    "",
    "/queue X         queue variable X",
    "/dequeue X       dequeue and compare top with X",
    "/empty <isempty> check whether queue is empty (<isempty> = true)",
    NULL
};

static char buffer[BUFSIZ];
static Queue *pQueue = NULL;
static TesterReturnValue convertReturnValue(QueueReturnID queueId);

TesterReturnValue TesterInitCallback()
{
    if (queueCreate(&pQueue, varDestroy))
        return TESTER_RETURN_MALLOC;
    return TESTER_RETURN_OK;
}

TesterReturnValue TesterParseCallback(const char *command)
{
    Variable *var, *var2;
    QueueReturnID queueId = QUEUE_RETURN_OK;
    if matches(command, "queue") {
        if (TesterParseArguments("s", buffer) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (varCreate(buffer, &var) != VAR_RETURN_OK)
            return TESTER_RETURN_MALLOC;
        if (queueId = queueQueue(pQueue, var))
            varDestroy(var);
    } else if matches(command, "dequeue") {
        if (TesterParseArguments("s", buffer) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (varCreate(buffer, &var) != VAR_RETURN_OK)
            return TESTER_RETURN_MALLOC;
        if (queueId = queueDequeue(pQueue, &var2)) {
            varDestroy(var);
        } else {
            int result;
            VarReturnID varId = varCompare(var, var2, &result);
            varDestroy(var);
            varDestroy(var2);
            if (varId || !result)
                return TESTER_RETURN_RETURN;
        }
    } else if matches(command, "empty") {
        int expected, obtained;
        if (TesterParseArguments("s", buffer) != 1)
            return TESTER_RETURN_ARGUMENT;
        expected = matches(buffer, "true");
        queueId = queueIsEmpty(pQueue, &obtained);
        if (queueId == QUEUE_RETURN_OK && expected != obtained)
            return TESTER_RETURN_RETURN;
    } else {
        return TESTER_RETURN_COMMAND;
    }
    return convertReturnValue(queueId);
}

TesterReturnValue TesterExitCallback()
{
    queueDestroy(pQueue);
    pQueue = NULL;
#ifdef _DEBUG
    if (varGetRefCount())
        return TESTER_RETURN_MEMLEAK;
#endif
    return TESTER_RETURN_OK;
}

static TesterReturnValue convertReturnValue(QueueReturnID queueId)
{
    switch (queueId) {
    case QUEUE_RETURN_OK:
        return TESTER_RETURN_OK;
    case QUEUE_RETURN_EMPTY:
        return TesterExternalReturnValue("empty");
    case QUEUE_RETURN_INVALID_PARAMETER:
        return TesterExternalReturnValue("parameter");
    case QUEUE_RETURN_MEMORY:
        return TesterExternalReturnValue("malloc");
    default:
        return TesterExternalReturnValue("unknown");
    }
}