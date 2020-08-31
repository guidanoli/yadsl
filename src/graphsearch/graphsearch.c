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
static GraphSearchRet dfs(yadsl_GraphHandle *graph,
	int visitedFlag,
	void *vertex,
	void (*visit_vertex_func)(void *vertex),
	void (*visit_edge_func)(void *source, void *edge, void *dest));

static GraphSearchRet bfs(yadsl_GraphHandle *graph,
	Queue *pBfsQueue,
	int visitedFlag,
	void *vertex,
	void (*visit_vertex_func)(void *vertex),
	void (*visit_edge_func)(void *source, void *edge, void *dest));

/* Public functions */
GraphSearchRet graphDFS(yadsl_GraphHandle *graph,
	void *initialVertex,
	int visitedFlag,
	void (*visit_vertex_func)(void *vertex),
	void (*visit_edge_func)(void *source, void *edge, void *dest))
{
	yadsl_GraphRet graphId;
	int flag;
	if (graphId = yadsl_graph_vertex_flag_get(graph, initialVertex, &flag)) {
		switch (graphId) {
		case YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX:
			return GRAPH_SEARCH_DOES_NOT_CONTAIN_VERTEX;
		default:
			assert(0);
		}
	}
	if (flag == visitedFlag)
		return GRAPH_SEARCH_VERTEX_ALREADY_VISITED;
	return dfs(graph,
		visitedFlag,
		initialVertex,
		visit_vertex_func,
		visit_edge_func);
}

GraphSearchRet graphBFS(yadsl_GraphHandle *graph,
	void *initialVertex,
	int visitedFlag,
	void (*visit_vertex_func)(void *vertex),
	void (*visit_edge_func)(void *source, void *edge, void *dest))
{
	Queue *pBfsQueue;
	QueueRet queueId;
	yadsl_GraphRet graphId;
	GraphSearchRet id;
	int flag;
	if (graphId = yadsl_graph_vertex_flag_get(graph, initialVertex, &flag)) {
		switch (graphId) {
		case YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX:
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
	id = bfs(graph,
		pBfsQueue,
		visitedFlag,
		initialVertex,
		visit_vertex_func,
		visit_edge_func);
	queueDestroy(pBfsQueue);
	return id;
}

/* Private functions */

// Run dfs on unvisited vertex
static GraphSearchRet dfs(yadsl_GraphHandle *graph,
	int visitedFlag,
	void *vertex,
	void (*visit_vertex_func)(void *vertex),
	void (*visit_edge_func)(void *source, void *edge, void *dest))
{
	GraphSearchRet id;
	void *neighbour, *edge;
	size_t degree;
	bool is_directed;
	int flag;
	if (visit_vertex_func)
		visit_vertex_func(vertex);
	if (yadsl_graph_vertex_flag_set(graph, vertex, visitedFlag)) assert(0);
	if (yadsl_graph_is_directed_check(graph, &is_directed)) assert(0);
	if (is_directed) {
		if (yadsl_graph_vertex_degree_get(graph, vertex, YADSL_GRAPH_EDGE_DIR_OUT, &degree)) assert(0);
		while (degree--) {
			if (yadsl_graph_vertex_nb_iter(graph, vertex, YADSL_GRAPH_EDGE_DIR_OUT, YADSL_GRAPH_ITER_DIR_NEXT, &neighbour, &edge))
				assert(0);
			if (yadsl_graph_vertex_flag_get(graph, neighbour, &flag)) assert(0);
			if (flag == visitedFlag)
				continue;
			if (visit_edge_func)
				visit_edge_func(vertex, edge, neighbour);
			if (id = dfs(graph,
				visitedFlag,
				neighbour,
				visit_vertex_func,
				visit_edge_func))
				return id;
		}
	} else {
		if (yadsl_graph_vertex_degree_get(graph, vertex, YADSL_GRAPH_EDGE_DIR_BOTH, &degree))
			assert(0);
		while (degree--) {
			if (yadsl_graph_vertex_nb_iter(graph, vertex, YADSL_GRAPH_EDGE_DIR_BOTH, YADSL_GRAPH_ITER_DIR_NEXT, &neighbour, &edge))
				assert(0);
			if (yadsl_graph_vertex_flag_get(graph, neighbour, &flag)) assert(0);
			if (flag == visitedFlag)
				continue;
			if (visit_edge_func)
				visit_edge_func(vertex, edge, neighbour);
			if (id = dfs(graph,
				visitedFlag,
				neighbour,
				visit_vertex_func,
				visit_edge_func))
				return id;
		}
	}
	return GRAPH_SEARCH_OK;
}

static GraphSearchRet addNeighboursToQueue(yadsl_GraphHandle *graph,
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
	if (yadsl_graph_vertex_degree_get(graph, vertex, edge_direction, &degree)) assert(0);
	while (degree--) {
		if (yadsl_graph_vertex_nb_iter(graph, vertex, edge_direction, YADSL_GRAPH_ITER_DIR_NEXT, &neighbour, &edge)) assert(0);
		if (yadsl_graph_vertex_flag_get(graph, neighbour, &flag)) assert(0);
		if (flag == visitedFlag)
			continue;
		if (yadsl_graph_vertex_flag_set(graph, neighbour, visitedFlag)) assert(0);
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
static GraphSearchRet bfs(yadsl_GraphHandle *graph,
	Queue *pBfsQueue,
	int visitedFlag,
	void *vertex,
	void (*visit_vertex_func)(void *vertex),
	void (*visit_edge_func)(void *source, void *edge, void *dest))
{
	bool is_directed;
	struct bfsTreeNode *node = NULL;
	GraphSearchRet id;
	yadsl_GraphEdgeDirection edge_direction;
	if (yadsl_graph_is_directed_check(graph, &is_directed)) assert(0);
	if (is_directed) {
		edge_direction = YADSL_GRAPH_EDGE_DIR_OUT;
	} else {
		edge_direction = YADSL_GRAPH_EDGE_DIR_BOTH;
	}
	if (yadsl_graph_vertex_flag_set(graph, vertex, visitedFlag)) assert(0);
	if (visit_vertex_func)
		visit_vertex_func(vertex);
	if (id = addNeighboursToQueue(graph, pBfsQueue, visitedFlag,
		vertex, edge_direction))
		return id;
	while (queueDequeue(pBfsQueue, &node) == QUEUE_OK) {
		if (visit_edge_func)
			visit_edge_func(node->pParent, node->pEdge, node->pChild);
		if (visit_vertex_func)
			visit_vertex_func(node->pChild);
		vertex = node->pChild;
		freeNode(node);
		if (id = addNeighboursToQueue(graph, pBfsQueue, visitedFlag,
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
