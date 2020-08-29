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
static GraphSearchRet dfs(yadsl_GraphHandle *pGraph,
	int visitedFlag,
	void *vertex,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest));

static GraphSearchRet bfs(yadsl_GraphHandle *pGraph,
	Queue *pBfsQueue,
	int visitedFlag,
	void *vertex,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest));

/* Public functions */
GraphSearchRet graphDFS(yadsl_GraphHandle *pGraph,
	void *initialVertex,
	int visitedFlag,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest))
{
	yadsl_GraphReturnCondition graphId;
	int flag;
	if (graphId = yadsl_graph_vertex_flag_get(pGraph, initialVertex, &flag)) {
		switch (graphId) {
		case YADSL_GRAPH_RET_COND_DOES_NOT_CONTAIN_VERTEX:
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

GraphSearchRet graphBFS(yadsl_GraphHandle *pGraph,
	void *initialVertex,
	int visitedFlag,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest))
{
	Queue *pBfsQueue;
	QueueRet queueId;
	yadsl_GraphReturnCondition graphId;
	GraphSearchRet id;
	int flag;
	if (graphId = yadsl_graph_vertex_flag_get(pGraph, initialVertex, &flag)) {
		switch (graphId) {
		case YADSL_GRAPH_RET_COND_DOES_NOT_CONTAIN_VERTEX:
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
static GraphSearchRet dfs(yadsl_GraphHandle *pGraph,
	int visitedFlag,
	void *vertex,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest))
{
	GraphSearchRet id;
	void *neighbour, *edge;
	size_t degree;
	bool is_directed;
	int flag;
	if (visitVertexCallback)
		visitVertexCallback(vertex);
	if (yadsl_graph_vertex_flag_set(pGraph, vertex, visitedFlag)) assert(0);
	if (yadsl_graph_is_directed_check(pGraph, &is_directed)) assert(0);
	if (is_directed) {
		if (yadsl_graph_vertex_degree_get(pGraph, vertex, YADSL_GRAPH_EDGE_DIR_OUT, &degree)) assert(0);
		while (degree--) {
			if (yadsl_graph_vertex_nb_iter(pGraph, vertex, YADSL_GRAPH_EDGE_DIR_OUT, YADSL_GRAPH_ITER_DIR_NEXT, &neighbour, &edge))
				assert(0);
			if (yadsl_graph_vertex_flag_get(pGraph, neighbour, &flag)) assert(0);
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
		if (yadsl_graph_vertex_degree_get(pGraph, vertex, YADSL_GRAPH_EDGE_DIR_BOTH, &degree))
			assert(0);
		while (degree--) {
			if (yadsl_graph_vertex_nb_iter(pGraph, vertex, YADSL_GRAPH_EDGE_DIR_BOTH, YADSL_GRAPH_ITER_DIR_NEXT, &neighbour, &edge))
				assert(0);
			if (yadsl_graph_vertex_flag_get(pGraph, neighbour, &flag)) assert(0);
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

static GraphSearchRet addNeighboursToQueue(yadsl_GraphHandle *pGraph,
	Queue *pBfsQueue,
	int visitedFlag,
	void *vertex,
	yadsl_GraphEdgeDirection edge_direction)
{
	struct bfsTreeNode *node = NULL;
	QueueRet queueId;
	void *neighbour, *edge;
	size_t degree;
	int flag;
	if (yadsl_graph_vertex_degree_get(pGraph, vertex, edge_direction, &degree)) assert(0);
	while (degree--) {
		if (yadsl_graph_vertex_nb_iter(pGraph, vertex, edge_direction, YADSL_GRAPH_ITER_DIR_NEXT, &neighbour, &edge)) assert(0);
		if (yadsl_graph_vertex_flag_get(pGraph, neighbour, &flag)) assert(0);
		if (flag == visitedFlag)
			continue;
		if (yadsl_graph_vertex_flag_set(pGraph, neighbour, visitedFlag)) assert(0);
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
static GraphSearchRet bfs(yadsl_GraphHandle *pGraph,
	Queue *pBfsQueue,
	int visitedFlag,
	void *vertex,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest))
{
	bool is_directed;
	struct bfsTreeNode *node = NULL;
	GraphSearchRet id;
	yadsl_GraphEdgeDirection edge_direction;
	if (yadsl_graph_is_directed_check(pGraph, &is_directed)) assert(0);
	if (is_directed) {
		edge_direction = YADSL_GRAPH_EDGE_DIR_OUT;
	} else {
		edge_direction = YADSL_GRAPH_EDGE_DIR_BOTH;
	}
	if (yadsl_graph_vertex_flag_set(pGraph, vertex, visitedFlag)) assert(0);
	if (visitVertexCallback)
		visitVertexCallback(vertex);
	if (id = addNeighboursToQueue(pGraph, pBfsQueue, visitedFlag,
		vertex, edge_direction))
		return id;
	while (queueDequeue(pBfsQueue, &node) == QUEUE_OK) {
		if (visitEdgeCallback)
			visitEdgeCallback(node->pParent, node->pEdge, node->pChild);
		if (visitVertexCallback)
			visitVertexCallback(node->pChild);
		vertex = node->pChild;
		freeNode(node);
		if (id = addNeighboursToQueue(pGraph, pBfsQueue, visitedFlag,
			vertex, edge_direction))
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
