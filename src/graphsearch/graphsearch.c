#include <graphsearch/graphsearch.h>

#include <assert.h>
#include <stdlib.h>

#include <queue/queue.h>
#include <memdb/memdb.h>

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
static GraphSearchRet dfs(Graph *pGraph,
	int visitedFlag,
	void *vertex,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest));

static GraphSearchRet bfs(Graph *pGraph,
	Queue *pBfsQueue,
	int visitedFlag,
	void *vertex,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest));

/* Public functions */
GraphSearchRet graphDFS(Graph *pGraph,
	void *initialVertex,
	int visitedFlag,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest))
{
	GraphRet graphId;
	int flag;
	if (graphId = graphGetVertexFlag(pGraph, initialVertex, &flag)) {
		switch (graphId) {
		case GRAPH_DOES_NOT_CONTAIN_VERTEX:
			return GRAPH_SEARCH_DOES_NOT_CONTAIN_VERTEX;
		default:
			assert(0);
		}
	}
	if (flag == visitedFlag)
		return GRAPH_SEARCH_VERTEX_ALREADY_VISITED;
	return dfs(pGraph,
		visitedFlag,
		initialVertex,
		visitVertexCallback,
		visitEdgeCallback);
}

GraphSearchRet graphBFS(Graph *pGraph,
	void *initialVertex,
	int visitedFlag,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest))
{
	Queue *pBfsQueue;
	QueueRet queueId;
	GraphRet graphId;
	GraphSearchRet id;
	int flag;
	if (graphId = graphGetVertexFlag(pGraph, initialVertex, &flag)) {
		switch (graphId) {
		case GRAPH_DOES_NOT_CONTAIN_VERTEX:
			return GRAPH_SEARCH_DOES_NOT_CONTAIN_VERTEX;
		default:
			assert(0);
		}
	}
	if (flag == visitedFlag)
		return GRAPH_SEARCH_VERTEX_ALREADY_VISITED;
	if (queueId = queueCreate(&pBfsQueue, freeNode)) {
		assert(queueId == QUEUE_MEMORY);
		return GRAPH_SEARCH_MEMORY;
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
static GraphSearchRet dfs(Graph *pGraph,
	int visitedFlag,
	void *vertex,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest))
{
	GraphSearchRet id;
	void *neighbour, *edge;
	size_t degree;
	int isDirected, flag;
	if (visitVertexCallback)
		visitVertexCallback(vertex);
	if (graphSetVertexFlag(pGraph, vertex, visitedFlag)) assert(0);
	if (graphIsDirected(pGraph, &isDirected)) assert(0);
	if (isDirected) {
		if (graphGetVertexOutDegree(pGraph, vertex, &degree)) assert(0);
		while (degree--) {
			if (graphGetNextOutNeighbour(pGraph, vertex, &neighbour, &edge))
				assert(0);
			if (graphGetVertexFlag(pGraph, neighbour, &flag)) assert(0);
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
		if (graphGetVertexDegree(pGraph, vertex, &degree)) assert(0);
		while (degree--) {
			if (graphGetNextNeighbour(pGraph, vertex, &neighbour, &edge))
				assert(0);
			if (graphGetVertexFlag(pGraph, neighbour, &flag)) assert(0);
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
	return GRAPH_SEARCH_OK;
}

static GraphSearchRet addNeighboursToQueue(Graph *pGraph,
	Queue *pBfsQueue,
	int visitedFlag,
	void *vertex,
	GraphRet(*getVertexDegree)(Graph *, void *, size_t *),
	GraphRet(*getVertexNeighbour)(Graph *, void *, void **, void **))
{
	struct bfsTreeNode *node = NULL;
	QueueRet queueId;
	void *neighbour, *edge;
	size_t degree;
	int flag;
	if (getVertexDegree(pGraph, vertex, &degree)) assert(0);
	while (degree--) {
		if (getVertexNeighbour(pGraph, vertex, &neighbour, &edge)) assert(0);
		if (graphGetVertexFlag(pGraph, neighbour, &flag)) assert(0);
		if (flag == visitedFlag)
			continue;
		if (graphSetVertexFlag(pGraph, neighbour, visitedFlag)) assert(0);
		node = allocNode(vertex, edge, neighbour);
		if (node == NULL)
			return GRAPH_SEARCH_MEMORY;
		if (queueId = queueQueue(pBfsQueue, node)) {
			freeNode(node);
			return GRAPH_SEARCH_MEMORY;
		}
	}
	return GRAPH_SEARCH_OK;
}

// Run bfs on unvisited vertex
static GraphSearchRet bfs(Graph *pGraph,
	Queue *pBfsQueue,
	int visitedFlag,
	void *vertex,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest))
{
	int isDirected;
	struct bfsTreeNode *node = NULL;
	GraphSearchRet id;
	GraphRet(*getVertexDegree)(Graph *, void *, size_t *);
	GraphRet(*getVertexNeighbour)(Graph *, void *, void **, void **);
	if (graphIsDirected(pGraph, &isDirected)) assert(0);
	if (isDirected) {
		getVertexDegree = graphGetVertexOutDegree;
		getVertexNeighbour = graphGetNextOutNeighbour;
	} else {
		getVertexDegree = graphGetVertexDegree;
		getVertexNeighbour = graphGetNextNeighbour;
	}
	if (graphSetVertexFlag(pGraph, vertex, visitedFlag)) assert(0);
	if (visitVertexCallback)
		visitVertexCallback(vertex);
	if (id = addNeighboursToQueue(pGraph, pBfsQueue, visitedFlag,
		vertex, getVertexDegree, getVertexNeighbour))
		return id;
	while (queueDequeue(pBfsQueue, &node) == QUEUE_OK) {
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
	return GRAPH_SEARCH_OK;
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
