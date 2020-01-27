#include <stdio.h>
#include <string.h>
#include "queue.h"
#include "var.h"

static char *helpStrings[] = {
    "This is the queue test module",
    "You will be interacting with the same queue at all times",
    "Actions are parsed by command line arguments",
    "The available actions are the following:",
    "",
    "X+\tqueue X",
    "-\tdequeue",
    NULL
};

static char *_parse(int *pArgIndex, QueueReturnID *pId, int cmdCount,
    char **cmds);

static Queue *pQueue = NULL;

int main(int argc, char **argv)
{
    QueueReturnID id;
    int argIndex = 0;
    char *err = NULL;
    if (argc == 1) {
        char **s = helpStrings;
        for (;*s;++s) puts(*s);
        return 0;
    }
    err = _parse(&argIndex, &id, argc, argv);
    if (pQueue)
        queueDestroy(pQueue);
#ifdef _DEBUG
    if (varGetRefCount())
        fprintf(stderr, "Memory leak detected\n");
#endif
    if (err) {
        if (id) {
            fprintf(stderr, "Error #%d on action #%d: %s\n",
                id, argIndex, err);
        } else {
            fprintf(stderr, "Error on action #%d: %s\n",
                argIndex, err);
        }
        return 1;
    } else {
        puts("No errors");
    }
    return 0;
}

static char *_parse(int *pArgIndex, QueueReturnID *pId, int cmdCount,
    char **cmds)
{
    Queue *tempQueue;
    Variable *var;
    char buffer[1024], end;
    if (*pId = queueCreate(&tempQueue, varDestroy))
        return "Could not create queue";
    pQueue = tempQueue;
    for (*pArgIndex = 1; *pArgIndex < cmdCount; ++(*pArgIndex)) {
        if (strcmp(cmds[*pArgIndex], "-") == 0) {
            if (*pId = queueDequeue(pQueue, &var))
                return "Could not dequeue";
            fprintf(stdout, "[%d] ", *pArgIndex);
            varWrite(var, stdout);
            fprintf(stdout, "\n");
            varDestroy(var);
        } else if (sscanf(cmds[*pArgIndex], "%[^+]+%c", buffer, &end) == 1) {
            if (varCreate(buffer, &var))
                return "Could not create variable";
            if (*pId = queueQueue(pQueue, var)) {
                varDestroy(var);
                return "Could not queue";
            }
        } else {
            return "Parsing error";
        }
    }
    return NULL;
}