#include "graphsearch.h"

#include <stddef.h>

#include "queue.h"

/* Private functions prototypes */
static GraphSearchReturnID dfs(Graph *pGraph,
	int visitedFlag,
	void *vertex,
	void (*visit_cb)(void *vertex));

static GraphSearchReturnID bfs(Graph *pGraph,
	Queue *pQueue,
	int visitedFlag,
	void *vertex,
	void (*visit_cb)(void *vertex));

/* Public functions */
GraphSearchReturnID graphDFS(Graph *pGraph,
	void *initialVertex,
	int visitedFlag,
	void (*visit_cb)(void *vertex))
{
	GraphReturnID graphId;
	if (pGraph == NULL || visit_cb == NULL)
		return GRAPH_SEARCH_RETURN_INVALID_PARAMETER;
	if ((graphId = graphContainsVertex(pGraph, initialVertex)) !=
		GRAPH_RETURN_CONTAINS_VERTEX) {
		if (graphId != GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_DOES_NOT_CONTAIN_VERTEX;
	}
	dfs(pGraph, visitedFlag, initialVertex, visit_cb);
	return GRAPH_SEARCH_RETURN_OK;
}

GraphSearchReturnID graphBFS(Graph *pGraph,
	void *initialVertex,
	int visitedFlag,
	void (*visit_cb)(void *vertex))
{
	Queue *pQueue;
	QueueReturnID queueId;
	GraphReturnID graphId;
	GraphSearchReturnID id;
	if (pGraph == NULL || visit_cb == NULL)
		return GRAPH_SEARCH_RETURN_INVALID_PARAMETER;
	if ((graphId = graphContainsVertex(pGraph, initialVertex)) !=
		GRAPH_RETURN_CONTAINS_VERTEX) {
		if (graphId != GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_DOES_NOT_CONTAIN_VERTEX;
	}
	if (queueId = queueCreate(&pQueue, NULL)) {
		if (queueId != QUEUE_RETURN_MEMORY)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_MEMORY;
	}
	id = bfs(pGraph, pQueue, visitedFlag, initialVertex, visit_cb);
	queueDestroy(pQueue);
	return id;
}

/* Private functions */
static GraphSearchReturnID dfs(Graph *pGraph,
	int visitedFlag,
	void *vertex,
	void (*visit_cb)(void *vertex))
{
	GraphSearchReturnID id;
	void *neighbour, *edge;
	unsigned long degree;
	int isDirected, flag;
	if (graphGetVertexFlag(pGraph, vertex, &flag))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
	if (flag == visitedFlag)
		return GRAPH_SEARCH_RETURN_OK;
	visit_cb(vertex);
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
			if (id = dfs(pGraph, visitedFlag, neighbour, visit_cb))
				return id;
		}
	} else {
		if (graphGetVertexDegree(pGraph, vertex, &degree))
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		while (degree--) {
			if (graphGetNextNeighbour(pGraph, vertex, &neighbour, &edge))
				return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
			if (id = dfs(pGraph, visitedFlag, neighbour, visit_cb))
				return id;
		}
	}
	return GRAPH_SEARCH_RETURN_OK;
}

static GraphSearchReturnID bfs(Graph *pGraph,
	Queue *pQueue,
	int visitedFlag,
	void *vertex,
	void (*visit_cb)(void *vertex))
{
	void *temp;
	int isDirected, flag;
	QueueReturnID queueId;
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
	if (graphGetVertexFlag(pGraph, vertex, &flag))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
	if (flag == visitedFlag)
		return GRAPH_SEARCH_RETURN_OK;
	queueQueue(pQueue, vertex);
	if (graphSetVertexFlag(pGraph, vertex, visitedFlag))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
	while (queueDequeue(pQueue, &vertex) != QUEUE_RETURN_EMPTY) {
		unsigned long degree;
		void *neighbour;
		visit_cb(vertex);
		if (getVertexDegree(pGraph, vertex, &degree))
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		while (degree--) {
			if (getVertexNeighbour(pGraph, vertex, &neighbour, &temp))
				return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
			if (graphGetVertexFlag(pGraph, neighbour, &flag))
				return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
			if (flag == visitedFlag)
				continue;
			if (queueId = queueQueue(pQueue, neighbour)) {
				if (queueId != QUEUE_RETURN_MEMORY)
					return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
				return GRAPH_SEARCH_RETURN_MEMORY;
			}
		}
	}
	return GRAPH_SEARCH_RETURN_OK;
}