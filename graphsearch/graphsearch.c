#include "graphsearch.h"

#include <stdlib.h>

#include <common/assert.h>

#include "queue.h"
#include "memdb.h"

#ifdef _DEBUG
#include <stdio.h>
int nodeRefCount = 0;
#endif

struct bfsTreeNode
{
	void *pParent;
	void *pEdge;
	void *pChild;
};

static struct bfsTreeNode *allocNode(void *parent, void *edge, void *child);
static void freeNode(struct bfsTreeNode *node);

/* Private functions prototypes */
static GraphSearchReturnID dfs(Graph *pGraph,
	int visitedFlag,
	void *vertex,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest));

static GraphSearchReturnID bfs(Graph *pGraph,
	Queue *pBfsQueue,
	int visitedFlag,
	void *vertex,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest));

/* Public functions */
GraphSearchReturnID graphDFS(Graph *pGraph,
	void *initialVertex,
	int visitedFlag,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest))
{
	GraphReturnID graphId;
	int flag;
	if (graphId = graphGetVertexFlag(pGraph, initialVertex, &flag)) {
		switch (graphId) {
		case GRAPH_RETURN_INVALID_PARAMETER:
			return GRAPH_SEARCH_RETURN_INVALID_PARAMETER;
		case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
			return GRAPH_SEARCH_RETURN_DOES_NOT_CONTAIN_VERTEX;
		default:
			_assert(0);
		}
	}
	if (flag == visitedFlag)
		return GRAPH_SEARCH_RETURN_VERTEX_ALREADY_VISITED;
	return dfs(pGraph,
		visitedFlag,
		initialVertex,
		visitVertexCallback,
		visitEdgeCallback);
}

GraphSearchReturnID graphBFS(Graph *pGraph,
	void *initialVertex,
	int visitedFlag,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest))
{
	Queue *pBfsQueue;
	QueueReturnID queueId;
	GraphReturnID graphId;
	GraphSearchReturnID id;
	int flag;
	if (graphId = graphGetVertexFlag(pGraph, initialVertex, &flag)) {
		switch (graphId) {
		case GRAPH_RETURN_INVALID_PARAMETER:
			return GRAPH_SEARCH_RETURN_INVALID_PARAMETER;
		case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
			return GRAPH_SEARCH_RETURN_DOES_NOT_CONTAIN_VERTEX;
		default:
			_assert(0);
		}
	}
	if (flag == visitedFlag)
		return GRAPH_SEARCH_RETURN_VERTEX_ALREADY_VISITED;
	if (queueId = queueCreate(&pBfsQueue, freeNode)) {
		_assert(queueId == QUEUE_RETURN_MEMORY);
		return GRAPH_SEARCH_RETURN_MEMORY;
	}
	id = bfs(pGraph,
		pBfsQueue,
		visitedFlag,
		initialVertex,
		visitVertexCallback,
		visitEdgeCallback);
	queueDestroy(pBfsQueue);
	return id;
}

/* Private functions */

// Run dfs on unvisited vertex
static GraphSearchReturnID dfs(Graph *pGraph,
	int visitedFlag,
	void *vertex,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest))
{
	GraphSearchReturnID id;
	void *neighbour, *edge;
	size_t degree;
	int isDirected, flag;
	if (visitVertexCallback)
		visitVertexCallback(vertex);
	_assert(!graphSetVertexFlag(pGraph, vertex, visitedFlag));
	_assert(!graphIsDirected(pGraph, &isDirected));
	if (isDirected) {
		_assert(!graphGetVertexOutDegree(pGraph, vertex, &degree));
		while (degree--) {
			_assert(!graphGetNextOutNeighbour(pGraph, vertex, &neighbour,
				&edge));
			_assert(!graphGetVertexFlag(pGraph, neighbour, &flag));
			if (flag == visitedFlag)
				continue;
			if (visitEdgeCallback)
				visitEdgeCallback(vertex, edge, neighbour);
			if (id = dfs(pGraph,
				visitedFlag,
				neighbour,
				visitVertexCallback,
				visitEdgeCallback))
				return id;
		}
	} else {
		_assert(!graphGetVertexDegree(pGraph, vertex, &degree));
		while (degree--) {
			_assert(!graphGetNextNeighbour(pGraph, vertex, &neighbour, &edge));
			_assert(!graphGetVertexFlag(pGraph, neighbour, &flag));
			if (flag == visitedFlag)
				continue;
			if (visitEdgeCallback)
				visitEdgeCallback(vertex, edge, neighbour);
			if (id = dfs(pGraph,
				visitedFlag,
				neighbour,
				visitVertexCallback,
				visitEdgeCallback))
				return id;
		}
	}
	return GRAPH_SEARCH_RETURN_OK;
}

static GraphSearchReturnID addNeighboursToQueue(Graph *pGraph,
	Queue *pBfsQueue,
	int visitedFlag,
	void *vertex,
	GraphReturnID(*getVertexDegree)(Graph *, void *, size_t *),
	GraphReturnID(*getVertexNeighbour)(Graph *, void *, void **, void **))
{
	struct bfsTreeNode *node = NULL;
	QueueReturnID queueId;
	void *neighbour, *edge;
	size_t degree;
	int flag;
	_assert(!getVertexDegree(pGraph, vertex, &degree));
	while (degree--) {
		_assert(!getVertexNeighbour(pGraph, vertex, &neighbour, &edge));
		_assert(!graphGetVertexFlag(pGraph, neighbour, &flag));
		if (flag == visitedFlag)
			continue;
		_assert(!graphSetVertexFlag(pGraph, neighbour, visitedFlag));
		node = allocNode(vertex, edge, neighbour);
		if (node == NULL)
			return GRAPH_SEARCH_RETURN_MEMORY;
		if (queueId = queueQueue(pBfsQueue, node)) {
			freeNode(node);
			_assert(queueId == QUEUE_RETURN_MEMORY);
			return GRAPH_SEARCH_RETURN_MEMORY;
		}
	}
	return GRAPH_SEARCH_RETURN_OK;
}

// Run bfs on unvisited vertex
static GraphSearchReturnID bfs(Graph *pGraph,
	Queue *pBfsQueue,
	int visitedFlag,
	void *vertex,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest))
{
	int isDirected;
	struct bfsTreeNode *node = NULL;
	GraphSearchReturnID id;
	GraphReturnID(*getVertexDegree)(Graph *, void *, size_t *);
	GraphReturnID(*getVertexNeighbour)(Graph *, void *, void **, void **);
	_assert(!graphIsDirected(pGraph, &isDirected));
	if (isDirected) {
		getVertexDegree = graphGetVertexOutDegree;
		getVertexNeighbour = graphGetNextOutNeighbour;
	} else {
		getVertexDegree = graphGetVertexDegree;
		getVertexNeighbour = graphGetNextNeighbour;
	}
	_assert(!graphSetVertexFlag(pGraph, vertex, visitedFlag));
	if (visitVertexCallback)
		visitVertexCallback(vertex);
	if (id = addNeighboursToQueue(pGraph, pBfsQueue, visitedFlag,
		vertex, getVertexDegree, getVertexNeighbour))
		return id;
	while (queueDequeue(pBfsQueue, &node) == QUEUE_RETURN_OK) {
		if (visitEdgeCallback)
			visitEdgeCallback(node->pParent, node->pEdge, node->pChild);
		if (visitVertexCallback)
			visitVertexCallback(node->pChild);
		vertex = node->pChild;
		freeNode(node);
		if (id = addNeighboursToQueue(pGraph, pBfsQueue, visitedFlag,
			vertex, getVertexDegree, getVertexNeighbour))
			return id;
	}
	return GRAPH_SEARCH_RETURN_OK;
}

static struct bfsTreeNode *allocNode(void *parent, void *edge, void *child)
{
	struct bfsTreeNode *node;
	node = (struct bfsTreeNode *) malloc(sizeof(struct bfsTreeNode));
	if (node) {
		node->pParent = parent;
		node->pEdge = edge;
		node->pChild = child;
#ifdef _DEBUG
		++nodeRefCount;
		printf("Allocating node %p\n", node);
#endif
	}
	return node;
}

static void freeNode(struct bfsTreeNode *node)
{
#ifdef _DEBUG
	--nodeRefCount;
	printf("Deallocating node %p\n", node);
#endif
	free(node);
}

#ifdef _DEBUG
int getGraphSearchNodeRefCount()
{
	return nodeRefCount;
}
#endif
