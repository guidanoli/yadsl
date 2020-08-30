#include <heap/heap.h>

#include <yadsl/posixstring.h>
#include <stdlib.h>

#include <tester/tester.h>

#define matches(a, b) (!strcmp(a,b))

const char *yadsl_tester_help_strings[] = {
    "This is the heap test module",
    "This implements a min heap, meaning that the first item extracted",
    "is the smallest of all in the heap",
    "",
    "/create <size>               create a heap with size <size>",
    "/insert <number>             insert <number> in the heap",
    "/extract <expected number>   extract number from heap",
    "/size <expected size>        get heap size",
    "/resize <new size>           resize heap to size <new size>",
    NULL,
};

static Heap *pHeap;

yadsl_TesterRet yadsl_tester_init()
{
    pHeap = NULL;
    return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet convert(HeapRet heapReturnValue)
{
    switch (heapReturnValue) {
    case HEAP_OK:
        return YADSL_TESTER_RET_OK;
    case HEAP_EMPTY:
        return yadsl_tester_return_external_value("empty");
    case HEAP_FULL:
        return yadsl_tester_return_external_value("full");
    case HEAP_SHRINK:
        return yadsl_tester_return_external_value("shrink");
    case HEAP_MEMORY:
        return yadsl_tester_return_external_value("memory");
    default:
        return yadsl_tester_return_external_value("unknown");
    }
}

int cmpObjs(void *a, void *b, void *_unused)
{
    return *((int *) a) < *((int *) b); /* MIN HEAP */
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
    HeapRet returnId = HEAP_OK;
    if matches(command, "create") {
        size_t size;
        Heap *temp;
        if (yadsl_tester_parse_arguments("z", &size) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        if (pHeap != NULL)
            heapDestroy(pHeap);
        returnId = heapCreate(&temp, size, cmpObjs, free, NULL);
        if (!returnId)
            pHeap = temp;
    } else if matches(command, "insert") {
        int obj, *pObj;
        if (yadsl_tester_parse_arguments("i", &obj) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        pObj = malloc(sizeof(int));
        if (!pObj)
            return YADSL_TESTER_RET_MALLOC;
        *pObj = obj;
        returnId = heapInsert(pHeap, pObj);
        if (returnId)
            free(pObj);
    } else if matches(command, "extract") {
        int *pObj, actual, expected;
        if (yadsl_tester_parse_arguments("i", &expected) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        returnId = heapExtract(pHeap, &pObj);
        if (!returnId) {
            actual = *pObj;
            free(pObj);
            if (actual != expected)
                return YADSL_TESTER_RET_RETURN;
        }
    } else if matches(command, "size") {
        size_t actual, expected;
        if (yadsl_tester_parse_arguments("z", &expected) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        returnId = heapGetSize(pHeap, &actual);
        if (!returnId && actual != expected)
            return YADSL_TESTER_RET_RETURN;
    } else if matches(command, "resize") {
        size_t newSize;
        if (yadsl_tester_parse_arguments("z", &newSize) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        returnId = heapResize(pHeap, newSize);
    } else {
        return YADSL_TESTER_RET_COMMAND;
    }
    return convert(returnId);
}

yadsl_TesterRet yadsl_tester_release()
{
    if (pHeap) {
        heapDestroy(pHeap);
        pHeap = NULL;
    }
    return YADSL_TESTER_RET_OK;
}

