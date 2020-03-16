#ifndef __QUEUE_H__
#define __QUEUE_H__

//
//     ____                      
//    / __ \__  _____  __  _____ 
//   / / / / / / / _ \/ / / / _ \
//  / /_/ / /_/ /  __/ /_/ /  __/
//  \___\_\__,_/\___/\__,_/\___/ 
//                              
//
//  A Queue starts empty. You can only queue and dequeue items.
// On destruction, freeItem is called for every remaining item still on queue.
//

typedef enum
{
	QUEUE_OK = 0,
	QUEUE_EMPTY,
	QUEUE_MEMORY,
}
QueueRet;

typedef struct Queue Queue;

//  ============= ================================== 
//   queueCreate        Create an empty queue        
//  ============= ================================== 
//   ppQueue       (ret) pointer to queue            
//   freeItem      (opt) item deallocation function  
//  ============= ================================== 
//  [!] QUEUE_MEMORY

QueueRet queueCreate(Queue **ppQueue,
	void (*freeItem)(void *item));

//  ============ =================== 
//   queueQueue      Queue item      
//  ============ =================== 
//   pQueue       pointer to queue   
//   item         item to be queued  
//  ============ =================== 
//  [!] QUEUE_MEMORY

QueueRet queueQueue(Queue *pQueue,
	void *item);

//  ============== ===================== 
//   queueDequeue      Dequeue item      
//  ============== ===================== 
//   pQueue         pointer to queue     
//   pItem          (ret) dequeued item  
//  ============== ===================== 
//  [!] QUEUE_EMPTY

QueueRet queueDequeue(Queue *pQueue,
	void **pItem);

//  ============== ====================================== 
//   queueIsEmpty   Checks whether queue is empty or not  
//  ============== ====================================== 
//   pQueue         pointer to queue                      
//   pIsEmpty       (ret) whether queue is empty or not   
//  ============== ====================================== 

QueueRet queueIsEmpty(Queue *pQueue,
	int *pIsEmpty);

//  ============== ======================================= 
//   queueDestroy   Destroy queue and its remaining items  
//  ============== ======================================= 
//   pQueue         pointer to queue                       
//  ============== ======================================= 

void queueDestroy(Queue *pQueue);

#endif