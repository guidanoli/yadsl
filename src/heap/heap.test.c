#include <heap/heap.h>

#include <string.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

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

static yadsl_HeapHandle *pHeap;

yadsl_TesterRet yadsl_tester_init()
{
    pHeap = NULL;
    return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet convert(yadsl_HeapRet heapReturnValue)
{
    switch (heapReturnValue) {
    case YADSL_HEAP_RET_OK:
        return YADSL_TESTER_RET_OK;
    case YADSL_HEAP_RET_EMPTY:
        return yadsl_tester_return_external_value("empty");
    case YADSL_HEAP_RET_FULL:
        return yadsl_tester_return_external_value("full");
    case YADSL_HEAP_RET_SHRINK:
        return yadsl_tester_return_external_value("shrink");
    case YADSL_HEAP_RET_MEMORY:
        return YADSL_TESTER_RET_MALLOC;
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
    yadsl_HeapRet returnId = YADSL_HEAP_RET_OK;
    if (yadsl_testerutils_match(command, "create")) {
        size_t size;
        yadsl_HeapHandle *temp;
        if (yadsl_tester_parse_arguments("z", &size) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        if (pHeap != NULL)
            yadsl_heap_destroy(pHeap);
        if (temp = yadsl_heap_create(size, cmpObjs, free, NULL))
            pHeap = temp;
        else
            return YADSL_TESTER_RET_MALLOC;
    } else if (yadsl_testerutils_match(command, "insert")) {
        int obj, *pObj;
        if (yadsl_tester_parse_arguments("i", &obj) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        pObj = malloc(sizeof(int));
        if (!pObj)
            return YADSL_TESTER_RET_MALLOC;
        *pObj = obj;
        returnId = yadsl_heap_insert(pHeap, pObj);
        if (returnId)
            free(pObj);
    } else if (yadsl_testerutils_match(command, "extract")) {
        int *pObj, actual, expected;
        if (yadsl_tester_parse_arguments("i", &expected) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        returnId = yadsl_heap_extract(pHeap, (yadsl_HeapObj**) &pObj);
        if (!returnId) {
            actual = *pObj;
            free(pObj);
            if (actual != expected)
                return YADSL_TESTER_RET_RETURN;
        }
    } else if (yadsl_testerutils_match(command, "size")) {
        size_t actual, expected;
        if (yadsl_tester_parse_arguments("z", &expected) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        returnId = yadsl_heap_size_get(pHeap, &actual);
        if (!returnId && actual != expected)
            return YADSL_TESTER_RET_RETURN;
    } else if (yadsl_testerutils_match(command, "resize")) {
        size_t newSize;
        if (yadsl_tester_parse_arguments("z", &newSize) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        returnId = yadsl_heap_resize(pHeap, newSize);
    } else {
        return YADSL_TESTER_RET_COMMAND;
    }
    return convert(returnId);
}

yadsl_TesterRet yadsl_tester_release()
{
    if (pHeap) {
        yadsl_heap_destroy(pHeap);
        pHeap = NULL;
    }
    return YADSL_TESTER_RET_OK;
}

