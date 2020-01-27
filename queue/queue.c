#include <stdlib.h>
#include "queue.h"

struct QueueItem
{
    struct QueueItem *next;
    void *item;
};

struct Queue
{
    struct QueueItem *top;
    void (*freeItem)(void *item);
};

QueueReturnID queueCreate(Queue **ppQueue,
    void (*freeItem)(void *item))
{
    Queue *pQueue;
    if (ppQueue == NULL)
        return QUEUE_RETURN_INVALID_PARAMETER;
    pQueue = malloc(sizeof(struct Queue));
    if (pQueue == NULL)
        return QUEUE_RETURN_MEMORY;
    pQueue->freeItem = freeItem;
    pQueue->top = NULL;
    *ppQueue = pQueue;
    return QUEUE_RETURN_OK;
}

QueueReturnID queueQueue(Queue *pQueue,
    void *item)
{
    struct QueueItem *pQueueItem;
    if (pQueue == NULL)
        return QUEUE_RETURN_INVALID_PARAMETER;
    pQueueItem = malloc(sizeof(struct QueueItem));
    if (pQueueItem == NULL)
        return QUEUE_RETURN_MEMORY;
    pQueueItem->item = item;
    pQueueItem->next = pQueue->top;
    pQueue->top = pQueueItem;
    return QUEUE_RETURN_OK;
}

QueueReturnID queueDequeue(Queue *pQueue,
    void **pItem)
{
    struct QueueItem *next;
    if (pQueue == NULL || pItem == NULL)
        return QUEUE_RETURN_OK;
    if (pQueue->top == NULL)
        return QUEUE_RETURN_EMPTY;
    *pItem = pQueue->top->item;
    next = pQueue->top->next;
    free(pQueue->top);
    pQueue->top = next;
    return QUEUE_RETURN_OK;
}

void queueDestroy(Queue *pQueue)
{
    struct QueueItem *old;
    if (pQueue == NULL)
        return;
    while (old = pQueue->top) {
        pQueue->top = old->next;
        if (pQueue->freeItem)
            pQueue->freeItem(old->item);
        free(old);
    }
    free(pQueue);
}