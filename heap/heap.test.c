#include "heap.h"

#include <string.h>

#include "tester.h"

#define matches(a, b) (!strcmp(a,b))

const char *TesterHelpStrings[] = {
    "This is the heap test module",
    "This implements a min heap",
    "",
    "/create <size>",
    "/insert <number>",
    "/extract <expected number>",
    "/resize <new size>",
    NULL,
};

static Heap *pHeap;

TesterReturnValue TesterInitCallback()
{
    pHeap = NULL;
    return TESTER_RETURN_OK;
}

TesterReturnValue convert(HeapReturnID heapReturnValue)
{
    switch (heapReturnValue) {
    case HEAP_RETURN_OK:
        return TESTER_RETURN_OK;
    case HEAP_RETURN_EMPTY:
        return TesterExternalReturnValue("empty");
    case HEAP_RETURN_FULL:
        return TesterExternalReturnValue("full");
    case HEAP_RETURN_SHRINK:
        return TesterExternalReturnValue("shrink");
    case HEAP_RETURN_INVALID_PARAMETER:
        return TesterExternalReturnValue("invalid parameter");
    case HEAP_RETURN_MEMORY:
        return TesterExternalReturnValue("memory");
    default:
        return TesterExternalReturnValue("unknown");
    }
}

int cmpObjs(void *a, void *b)
{
    return *((int *) a) < *((int *) b);
}

TesterReturnValue TesterParseCallback(const char *command)
{
    HeapReturnID returnId = HEAP_RETURN_OK;
    if matches(command, "create") {
        size_t size;
        Heap *temp;
        if (TesterParseArguments("z", &size) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (pHeap != NULL)
            heapDestroy(pHeap);
        returnId = heapCreate(&temp, size, cmpObjs, TesterFree);
        if (!returnId)
            pHeap = temp;
    } else if matches(command, "insert") {
        int obj, *pObj;
        if (TesterParseArguments("i", &obj) != 1)
            return TESTER_RETURN_ARGUMENT;
        pObj = TesterMalloc(sizeof(int));
        if (!pObj)
            return TESTER_RETURN_MALLOC;
        *pObj = obj;
        returnId = heapInsert(pHeap, pObj);
        if (returnId)
            TesterFree(pObj);
    } else if matches(command, "extract") {
        int *pObj, actual, expected;
        if (TesterParseArguments("i", &expected) != 1)
            return TESTER_RETURN_ARGUMENT;
        returnId = heapExtract(pHeap, &pObj);
        if (!returnId) {
            actual = *pObj;
            TesterFree(pObj);
            if (actual != expected)
                return TESTER_RETURN_RETURN;
        }
    } else if matches(command, "resize") {
        size_t newSize;
        if (TesterParseArguments("z", &newSize) != 1)
            return TESTER_RETURN_ARGUMENT;
        returnId = heapResize(pHeap, newSize);
    } else {
        return TESTER_RETURN_COMMAND;
    }
    return convert(returnId);
}

TesterReturnValue TesterExitCallback()
{
    if (pHeap)
        heapDestroy(pHeap);
    return TESTER_RETURN_OK;
}

