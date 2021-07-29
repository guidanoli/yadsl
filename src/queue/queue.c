#include <queue/queue.h>

#include <yadsl/stdlib.h>

struct yadsl_QueueItem_s
{
	struct yadsl_QueueItem_s* previous;
	yadsl_QueueItemObj* item;
};

typedef struct yadsl_QueueItem_s yadsl_QueueItem;

typedef struct
{
	yadsl_QueueItem* begin;
	yadsl_QueueItem* end;
	yadsl_QueueItemFreeFunc free_item_func;
}
yadsl_Queue;

yadsl_QueueHandle*
yadsl_queue_create(
	yadsl_QueueItemFreeFunc free_item_func)
{
	yadsl_Queue* queue = malloc(sizeof(*queue));
	if (queue) {
		queue->free_item_func = free_item_func;
		queue->begin = NULL;
		queue->end = NULL;
	}
	return queue;
}

yadsl_QueueRet
yadsl_queue_queue(
	yadsl_QueueHandle* queue,
	yadsl_QueueItemObj* item)
{
	yadsl_Queue* queue_ = (yadsl_Queue*) queue;
	yadsl_QueueItem* queue_item = malloc(sizeof(*queue_item));
	if (queue_item == NULL)
		return YADSL_QUEUE_RET_MEMORY;
	queue_item->item = item;
	queue_item->previous = NULL;
	if (queue_->begin != NULL)
		queue_->begin->previous = queue_item;
	queue_->begin = queue_item;
	if (queue_->end == NULL)
		queue_->end = queue_item;
	return YADSL_QUEUE_RET_OK;
}

yadsl_QueueRet
yadsl_queue_dequeue(
	yadsl_QueueHandle* queue,
	yadsl_QueueItemObj** item_ptr)
{
	yadsl_QueueItem* new_end;
	yadsl_Queue* queue_ = (yadsl_Queue*) queue;
	if (queue_->end == NULL)
		return YADSL_QUEUE_RET_EMPTY;
	new_end = queue_->end->previous;
	*item_ptr = queue_->end->item;
	free(queue_->end);
	queue_->end = new_end;
	if (new_end == NULL)
		queue_->begin = NULL;
	return YADSL_QUEUE_RET_OK;
}

yadsl_QueueRet
yadsl_queue_empty_check(
	yadsl_QueueHandle* queue,
	bool* is_empty_ptr)
{
	*is_empty_ptr = ((yadsl_Queue*) queue)->end == NULL;
	return YADSL_QUEUE_RET_OK;
}

void
yadsl_queue_destroy(
	yadsl_QueueHandle* queue)
{
	yadsl_QueueItem* current;
	yadsl_Queue* queue_ = (yadsl_Queue*) queue;
	while (current = queue_->end) {
		queue_->end = current->previous;
		if (queue_->free_item_func)
			queue_->free_item_func(current->item);
		free(current);
	}
	free(queue_);
}
