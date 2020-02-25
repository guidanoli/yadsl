#include "queue.h"

#include <stdlib.h>

#ifdef _DEBUG
#include "memdb.h"
#endif

struct QueueItem
{
	struct QueueItem *previous;
	void *item;
};

struct Queue
{
	struct QueueItem *entrance;
	struct QueueItem *exit;
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
	pQueue->entrance = NULL;
	pQueue->exit = NULL;
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
	pQueueItem->previous = NULL;
	if (pQueue->entrance != NULL)
		pQueue->entrance->previous = pQueueItem;
	pQueue->entrance = pQueueItem;
	if (pQueue->exit == NULL)
		pQueue->exit = pQueueItem;
	return QUEUE_RETURN_OK;
}

QueueReturnID queueDequeue(Queue *pQueue,
	void **pItem)
{
	struct QueueItem *newExit;
	if (pQueue == NULL || pItem == NULL)
		return QUEUE_RETURN_INVALID_PARAMETER;
	if (pQueue->exit == NULL)
		return QUEUE_RETURN_EMPTY;
	newExit = pQueue->exit->previous;
	*pItem = pQueue->exit->item;
	free(pQueue->exit);
	pQueue->exit = newExit;
	if (newExit == NULL)
		pQueue->entrance = NULL;
	return QUEUE_RETURN_OK;
}

QueueReturnID queueIsEmpty(Queue *pQueue,
	int *pIsEmpty)
{
	if (pQueue == NULL || pIsEmpty == NULL)
		return QUEUE_RETURN_INVALID_PARAMETER;
	*pIsEmpty = pQueue->exit == NULL;
	return QUEUE_RETURN_OK;
}

void queueDestroy(Queue *pQueue)
{
	struct QueueItem *current;
	if (pQueue == NULL)
		return;
	while (current = pQueue->exit) {
		pQueue->exit = current->previous;
		if (pQueue->freeItem)
			pQueue->freeItem(current->item);
		free(current);
	}
	free(pQueue);
}