#include "graphsearch.h"

#include <stdio.h>

#include "map.h"
#include "queue.h"

/* Private functions prototypes */
static GraphSearchReturnID dfs(Graph *pGraph,
	Map *pMap,
	void *vertex,
	void (*visit_cb)(void *vertex));

static GraphSearchReturnID bfs(Graph *pGraph,
	Map *pMap,
	Queue *pQueue,
	void *vertex,
	void (*visit_cb)(void *vertex));

/* Public functions */
GraphSearchReturnID graphDFS(Graph *pGraph,
	void *initialVertex,
	void (*visit_cb)(void *vertex))
{
	Map *pMap;
	MapReturnID mapId;
	GraphReturnID graphId;
	int (*cmpVertices)(void *a, void *b);
	if (pGraph == NULL || visit_cb == NULL)
		return GRAPH_SEARCH_RETURN_INVALID_PARAMETER;
	if ((graphId = graphContainsVertex(pGraph, initialVertex)) !=
		GRAPH_RETURN_CONTAINS_VERTEX) {
		if (graphId != GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_DOES_NOT_CONTAIN_VERTEX;
	}
	if (graphGetVertexComparisonFunc(pGraph, &cmpVertices))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
	if (mapId = mapCreate(&pMap, cmpVertices, NULL, NULL)) {
		if (mapId != MAP_RETURN_MEMORY)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_MEMORY;
	}
	dfs(pGraph, pMap, initialVertex, visit_cb);
	mapDestroy(pMap);
	return GRAPH_SEARCH_RETURN_OK;
}

GraphSearchReturnID graphBFS(Graph *pGraph,
	void *initialVertex,
	void (*visit_cb)(void *vertex))
{
	Map *pMap;
	MapReturnID mapId;
	Queue *pQueue;
	QueueReturnID queueId;
	GraphReturnID graphId;
	GraphSearchReturnID id;
	int (*cmpVertices)(void *a, void *b);
	if (pGraph == NULL || visit_cb == NULL)
		return GRAPH_SEARCH_RETURN_INVALID_PARAMETER;
	if ((graphId = graphContainsVertex(pGraph, initialVertex)) !=
		GRAPH_RETURN_CONTAINS_VERTEX) {
		if (graphId != GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_DOES_NOT_CONTAIN_VERTEX;
	}
	if (graphGetVertexComparisonFunc(pGraph, &cmpVertices))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
	if (mapId = mapCreate(&pMap, cmpVertices, NULL, NULL)) {
		if (mapId != MAP_RETURN_MEMORY)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_MEMORY;
	}
	if (queueId = queueCreate(&pQueue, NULL)) {
		mapDestroy(pMap);
		if (queueId != QUEUE_RETURN_MEMORY)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_MEMORY;
	}
	id = bfs(pGraph, pMap, pQueue, initialVertex, visit_cb);
	mapDestroy(pMap);
	queueDestroy(pQueue);
	return id;
}

/* Private functions */
static GraphSearchReturnID dfs(Graph *pGraph,
	Map *pMap,
	void *vertex,
	void (*visit_cb)(void *vertex))
{
	MapReturnID mapId;
	GraphSearchReturnID id;
	void *temp, *neighbour, *edge;
	unsigned long degree;
	int isDirected;
	if ((mapId = mapGetEntry(pMap, vertex, &temp)) !=
		MAP_RETURN_ENTRY_NOT_FOUND) {
		if (mapId != MAP_RETURN_OK)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_OK;
	}
	visit_cb(vertex);
	if (mapId = mapPutEntry(pMap, vertex, vertex, &temp)) {
		if (mapId != MAP_RETURN_MEMORY)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_MEMORY;
	}
	if (graphIsDirected(pGraph, &isDirected))
		return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
	if (isDirected) {
		if (graphGetVertexOutDegree(pGraph, vertex, &degree))
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		while (degree--) {
			if (graphGetNextOutNeighbour(pGraph, vertex, &neighbour, &edge))
				return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
			if (id = dfs(pGraph, pMap, neighbour, visit_cb))
				return id;
		}
	} else {
		if (graphGetVertexDegree(pGraph, vertex, &degree))
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		while (degree--) {
			if (graphGetNextNeighbour(pGraph, vertex, &neighbour, &edge))
				return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
			if (id = dfs(pGraph, pMap, neighbour, visit_cb))
				return id;
		}
	}
	return GRAPH_SEARCH_RETURN_OK;
}

static GraphSearchReturnID bfs(Graph *pGraph,
	Map *pMap,
	Queue *pQueue,
	void *vertex,
	void (*visit_cb)(void *vertex))
{
	void *temp;
	int isDirected;
	MapReturnID mapId;
	QueueReturnID queueId;
	GraphReturnID (*getVertexDegree)(Graph *, void *, unsigned long*);
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
	queueQueue(pQueue, vertex);
	if (mapId = mapPutEntry(pMap, vertex, vertex, &temp)) {
		if (mapId != MAP_RETURN_MEMORY)
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		return GRAPH_SEARCH_RETURN_MEMORY;
	}
	while (queueDequeue(pQueue, &vertex) != QUEUE_RETURN_EMPTY) {
		unsigned long degree;
		void *neighbour;
		visit_cb(vertex); /* visit vertex */
		if (getVertexDegree(pGraph, vertex, &degree))
			return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
		while (degree--) {
			if (getVertexNeighbour(pGraph, vertex, &neighbour, &temp))
				return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
			if (mapId = mapPutEntry(pMap, neighbour, neighbour, &temp)) {
				if (mapId == MAP_RETURN_OVERWROTE_ENTRY)
					continue; /* vertex already visited */
				if (mapId != MAP_RETURN_MEMORY)
					return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
				return GRAPH_SEARCH_RETURN_MEMORY;
			}
			if (queueId = queueQueue(pQueue, neighbour)) {
				if (queueId != QUEUE_RETURN_MEMORY)
					return GRAPH_SEARCH_RETURN_UNKNOWN_ERROR;
				return GRAPH_SEARCH_RETURN_MEMORY;
			}
		}
	}
	return GRAPH_SEARCH_RETURN_OK;
}