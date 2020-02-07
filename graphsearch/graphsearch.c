#include "graphsearch.h"

#include <stdlib.h>

#include "queue.h"

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
	if (pGraph == NULL)
		return GRAPH_SEARCH_RETURN_INVALID_PARAMETER;
	if ((graphId = graphContainsVertex(pGraph, initialVertex)) !=
		GRAPH_RETURN_CONTAINS_VERTEX) {
		if (graphId != GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_DOES_NOT_CONTAIN_VERTEX;
	}
	if (graphGetVertexFlag(pGraph, initialVertex, &flag))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
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
	if (pGraph == NULL)
		return GRAPH_SEARCH_RETURN_INVALID_PARAMETER;
	if ((graphId = graphContainsVertex(pGraph, initialVertex)) !=
		GRAPH_RETURN_CONTAINS_VERTEX) {
		if (graphId != GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_DOES_NOT_CONTAIN_VERTEX;
	}
	if (graphGetVertexFlag(pGraph, initialVertex, &flag))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
	if (flag == visitedFlag)
		return GRAPH_SEARCH_RETURN_VERTEX_ALREADY_VISITED;
	if (queueId = queueCreate(&pBfsQueue, freeNode)) {
		if (queueId != QUEUE_RETURN_MEMORY)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
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
	unsigned long degree;
	int isDirected, flag;
	if (visitVertexCallback)
		visitVertexCallback(vertex);
	if (graphSetVertexFlag(pGraph, vertex, visitedFlag))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
	if (graphIsDirected(pGraph, &isDirected))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
	if (isDirected) {
		if (graphGetVertexOutDegree(pGraph, vertex, &degree))
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		while (degree--) {
			if (graphGetNextOutNeighbour(pGraph, vertex, &neighbour, &edge))
				return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
			if (graphGetVertexFlag(pGraph, neighbour, &flag))
				return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
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
		if (graphGetVertexDegree(pGraph, vertex, &degree))
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		while (degree--) {
			if (graphGetNextNeighbour(pGraph, vertex, &neighbour, &edge))
				return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
			if (graphGetVertexFlag(pGraph, neighbour, &flag))
				return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
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
	GraphReturnID (*getVertexDegree)(Graph *, void *, unsigned long *),
	GraphReturnID (*getVertexNeighbour)(Graph *, void *, void **, void **))
{
	struct bfsTreeNode *node = NULL;
	QueueReturnID queueId;
	void *neighbour, *edge;
	unsigned long degree;
	int flag;
	if (getVertexDegree(pGraph, vertex, &degree))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
	while (degree--) {
		if (getVertexNeighbour(pGraph, vertex, &neighbour, &edge))
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		if (graphGetVertexFlag(pGraph, neighbour, &flag))
				return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		if (flag == visitedFlag)
			continue;
		if (graphSetVertexFlag(pGraph, neighbour, visitedFlag))
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		node = allocNode(vertex, edge, neighbour);
		if (node == NULL)
			return GRAPH_SEARCH_RETURN_MEMORY;
		if (queueId = queueQueue(pBfsQueue, node)) {
			freeNode(node);
			if (queueId != QUEUE_RETURN_MEMORY)
				return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
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
	GraphReturnID (*getVertexDegree)(Graph *, void *, unsigned long *);
	GraphReturnID (*getVertexNeighbour)(Graph *, void *, void **, void **);
	if (graphIsDirected(pGraph, &isDirected))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
	if (isDirected) {
		getVertexDegree = graphGetVertexOutDegree;
		getVertexNeighbour = graphGetNextOutNeighbour;
	} else {
		getVertexDegree = graphGetVertexDegree;
		getVertexNeighbour = graphGetNextNeighbour;
	}
	if (graphSetVertexFlag(pGraph, vertex, visitedFlag))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
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