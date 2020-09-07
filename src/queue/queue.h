#ifndef __YADSL_QUEUE_H__
#define __YADSL_QUEUE_H__

/**
 * \defgroup queue Queue
 * @brief Generic queue
 * 
 * A Queue starts empty. You can only queue and dequeue items.
 * On destruction, the items still on the queue are deallocated
 * with a function provided upon creation.
 * 
 * @{
*/

#include <stdbool.h>

/**
 * @brief Value returned by Queue functions
*/
typedef enum
{
	YADSL_QUEUE_RET_OK = 0, /**< All went ok*/
	YADSL_QUEUE_RET_EMPTY, /**< Queue is empty */
	YADSL_QUEUE_RET_MEMORY, /**< Could not allocate memory */
}
yadsl_QueueRet;

typedef void yadsl_QueueHandle; /**< Queue handle */
typedef void yadsl_QueueItemObj; /**< Queue item object */

/**
 * @brief Function responsible for freeing item
 * @param item queue item
*/
typedef void
(*yadsl_QueueItemFreeFunc)(
	yadsl_QueueItemObj* item);

/**
 * @brief Create an empty queue
 * @param free_item_func item freeing function
 * @return newly created queue or NULL if could not allocate memory
*/
yadsl_QueueHandle*
yadsl_queue_create(
	yadsl_QueueItemFreeFunc free_item_func);

/**
 * @brief Queue item
 * @param queue queue
 * @param item item to be queued
 * @return
 * * ::YADSL_QUEUE_RET_OK, and item is queued
 * * ::YADSL_QUEUE_RET_MEMORY
*/
yadsl_QueueRet
yadsl_queue_queue(
	yadsl_QueueHandle* queue,
	yadsl_QueueItemObj* item);

/**
 * @brief Dequeue item
 * @param queue queue
 * @param item_ptr dequeued item
 * @return
 * * ::YADSL_QUEUE_RET_OK, and *item_ptr is updated
 * * ::YADSL_QUEUE_RET_EMPTY
*/
yadsl_QueueRet
yadsl_queue_dequeue(
	yadsl_QueueHandle *queue,
	yadsl_QueueItemObj** item_ptr);

/**
 * @brief Check whether queue is empty or not
 * @param queue queue
 * @param is_empty_ptr whether queue is empty or not
 * @return
 * * ::YADSL_QUEUE_RET_OK, and *is_empty_ptr is updated
*/
yadsl_QueueRet
yadsl_queue_empty_check(
	yadsl_QueueHandle *queue,
	bool *is_empty_ptr);

/**
 * @brief Destroy queue and its remaining items
 * @param queue queue
*/
void
yadsl_queue_destroy(
	yadsl_QueueHandle *queue);

/** @} */

#endif