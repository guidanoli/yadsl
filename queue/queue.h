#ifndef __QUEUE_H__
#define __QUEUE_H__

/**
* A queue starts empty.
* You can only queue and dequeue items.
* On destruction, freeItem is called for
* every remaining item still on queue.
*/

typedef enum
{
	// Ok
	QUEUE_RETURN_OK = 0,

	// Queue is empty
	QUEUE_RETURN_EMPTY,

	// Unexpected parameter value
	QUEUE_RETURN_INVALID_PARAMETER,

	// Could not allocate memory space
	QUEUE_RETURN_MEMORY,
}
QueueReturnID;

typedef struct Queue Queue;

/**
* Creates an empty queue
* ppQueue   (return) pointer to queue
* freeItem  function responsible for
*           freeing items when the queue
*           is destroyed. If NULL, then
*           function will not be called.
* Possible errors:
* QUEUE_RETURN_INVALID_PARAMETER
* 	- "ppQueue" is NULL
* QUEUE_RETURN_MEMORY
*/
QueueReturnID queueCreate(Queue **ppQueue,
	void (*freeItem)(void *item));

/**
* Queue item
* pQueue    pointer to queue
* item      item to be queued
* Possible errors:
* QUEUE_RETURN_INVALID_PARAMETER
* 	- "pQueue" is NULL
* QUEUE_RETURN_MEMORY
*/
QueueReturnID queueQueue(Queue *pQueue,
	void *item);

/**
* Dequeue item
* pQueue    pointer to queue
* pItem	    (return) item from top
* Possible errors:
* QUEUE_RETURN_INVALID_PARAMETER
* 	- "pQueue" is NULL
* QUEUE_RETURN_EMPTY
* 	- no item to be dequeued
*/
QueueReturnID queueDequeue(Queue *pQueue,
	void **pItem);

/**
* Checks whether queue is empty or not
* pQueue    pointer to queue
* pIsEmpty  (return) is empty
* Possible errors:
* QUEUE_RETURN_INVALID_PARAMETER
* 	- "pQueue" is NULL
* 	- "pIsEmpty" is NULL
*/
QueueReturnID queueIsEmpty(Queue *pQueue,
	int *pIsEmpty);

/**
* Destroy queue and items left
* pQueue    pointer to queue
*/
void queueDestroy(Queue *pQueue);

#endif