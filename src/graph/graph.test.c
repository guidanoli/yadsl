#include <yadsl/posixstring.h>
#include <graph/graph.h>
#include <graphio/graphio.h>
#include <graphsearch/graphsearch.h>

#include <stdlib.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

/* Help */

const char* TesterHelpStrings[] = {
	"This is an interactive module of the graph library",
	"You will interact with the same graph object at all times",
	"A directed graph is already created from the start",
	"The registered actions are the following:",
	"",
	"Graph commands:",
	"/create [DIRECTED/UNDIRECTED]                           create new graph",
	"/isdirected [YES/NO]                                    check if graph is directed",
	"/vertexcount <expected>                                 get graph vertex count",
	"/ivertices <iter-dir> <expected>                        iterate through vertices",
	"/degree <vertex> <edge-dir> <expected>                  get vertex degree",
	"/ineighbours <vertex> <iter-dir> <edge-dir> <nb> <edge> iterate through neighbours",
	"/containsvertex <vertex> [YES/NO]                       check if graph contains vertex",
	"/addvertex <vertex>                                     add vertex to graph",
	"/removevertex <vertex>                                  remove vertex from graph",
	"/containsedge <u> <v> [YES/NO]                          check if graph contains edge",
	"/addedge <u> <v> <edge>                                 add edge to graph",
	"/getedge <u> <v> <edge>                                 get edge from graph",
	"/removeedege <u> <v>                                    remove edge uv",
	"/setvertexflag <u> <flag>                               set vertex flag",
	"/setallflags <flag>                                     set flag of all vertices",
	"/getvertexflag <u> <expected>                           get vertex flag",
	"",
	"Graph IO commands:",
	"/write <filename>                      write graph to file",
	"/read <filename>                       read from file to graph",
	"",
	"Graph Search commands:",
	"/dfs <v> <flag>                        run dfs on graph starting from v"
	"                                       v and marking visited with flag",
	"/bfs <v> <flag>                        run bfs on graph starting from v"
	"                                       v and marking visited with flag",
	NULL,
};

/* Graph */

static yadsl_GraphHandle* pGraph = NULL;
static char buffer[BUFSIZ], buffer2[BUFSIZ], buffer3[BUFSIZ],
buffer4[BUFSIZ], buffer5[BUFSIZ];

static int _cmpStrings(void* a, void* b);
static int _readString(FILE* fp, void** ppVertex);
static int _writeString(FILE* fp, void* v);

static bool parseEdgeDirection(const char* buffer, yadsl_GraphEdgeDirection* edge_direction_ptr);
static bool parseIterationDirection(const char* buffer, yadsl_GraphIterationDirection* iteration_direction_ptr);

static TesterReturnValue convertRet(yadsl_GraphReturnCondition graphId);
static TesterReturnValue convertIoRet(GraphIoRet graphIoId);
static TesterReturnValue convertSearchRet(GraphSearchRet graphSearchId);

TesterReturnValue TesterInitCallback()
{
	pGraph = yadsl_graph_create(1, _cmpStrings, free, _cmpStrings, free);
	return pGraph ? TESTER_OK : TESTER_MALLOC;
}

static TesterReturnValue parseGraphCommands(const char* command)
{
	yadsl_GraphReturnCondition graphId = YADSL_GRAPH_RET_COND_OK;
	if matches(command, "create") {
		int is_directed;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		is_directed = matches(buffer, "DIRECTED");
		yadsl_GraphHandle* temp = yadsl_graph_create(
			is_directed, _cmpStrings, free, _cmpStrings, free);
		if (temp) {
			yadsl_graph_destroy(pGraph);
			pGraph = temp;
		} else {
			return TESTER_MALLOC;
		}
	} else if matches(command, "isdirected") {
		bool actual, expected;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		expected = matches(buffer, "YES");
		graphId = yadsl_graph_is_directed_check(pGraph, &actual);
		if (graphId == YADSL_GRAPH_RET_COND_OK && expected != actual)
			return TESTER_RETURN;
	} else if matches(command, "vertexcount") {
		size_t actual, expected;
		if (TesterParseArguments("z", &expected) != 1)
			return TESTER_ARGUMENT;
		graphId = yadsl_graph_vertex_count_get(pGraph, &actual);
		if (graphId == YADSL_GRAPH_RET_COND_OK && expected != actual)
			return TESTER_RETURN;
	} else if matches(command, "ivertices") {
		char* vertex;
		yadsl_GraphIterationDirection iteration_direction;
		if (TesterParseArguments("ss", buffer, buffer2) != 2)
			return TESTER_ARGUMENT;
		if (!parseIterationDirection(buffer, &iteration_direction))
			return TESTER_ARGUMENT;
		graphId = yadsl_graph_vertex_iter(pGraph, iteration_direction, &vertex);
		if (graphId == YADSL_GRAPH_RET_COND_OK) {
			if (nmatches(buffer2, vertex))
				return TESTER_RETURN;
		}
	} else if matches(command, "degree") {
		size_t actual, expected;
		yadsl_GraphEdgeDirection edge_direction;
		if (TesterParseArguments("ssz", buffer, buffer2, &expected) != 3)
			return TESTER_ARGUMENT;
		if (!parseEdgeDirection(buffer2, &edge_direction))
			return TESTER_ARGUMENT;
		graphId = yadsl_graph_vertex_degree_get(pGraph, buffer, edge_direction, &actual);
		if (graphId == YADSL_GRAPH_RET_COND_OK && expected != actual)
			return TESTER_RETURN;
	} else if matches(command, "ineighbours") {
		char* v, * uv;
		yadsl_GraphIterationDirection iteration_direction;
		yadsl_GraphEdgeDirection edge_direction;
		if (TesterParseArguments("sssss", buffer, buffer2, buffer3, buffer4, buffer5) != 5)
			return TESTER_ARGUMENT;
		if (!parseIterationDirection(buffer2, &iteration_direction))
			return TESTER_ARGUMENT;
		if (!parseEdgeDirection(buffer3, &edge_direction))
			return TESTER_ARGUMENT;
		graphId = yadsl_graph_vertex_nb_iter(pGraph, buffer, edge_direction, iteration_direction, &v, &uv);
		if (graphId == YADSL_GRAPH_RET_COND_OK) {
			if (nmatches(v, buffer4) || nmatches(uv, buffer5))
				return TESTER_RETURN;
		}
	} else if matches(command, "containsvertex") {
		bool actual, expected;
		if (TesterParseArguments("ss", buffer, buffer2) != 2)
			return TESTER_ARGUMENT;
		expected = TesterUtilsGetYesOrNoFromString(buffer2);
		graphId = yadsl_graph_vertex_exists_check(pGraph, buffer, &actual);
		if (graphId == YADSL_GRAPH_RET_COND_OK && actual != expected)
			return TESTER_RETURN;
	} else if matches(command, "addvertex") {
		char* vertex;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		if ((vertex = strdup(buffer)) == NULL)
			return TESTER_MALLOC;
		if (graphId = yadsl_graph_vertex_add(pGraph, vertex))
			free(vertex);
	} else if matches(command, "removevertex") {
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		graphId = yadsl_graph_vertex_remove(pGraph, buffer);
	} else if matches(command, "containsedge") {
		bool actual, expected;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_ARGUMENT;
		expected = TesterUtilsGetYesOrNoFromString(buffer3);
		graphId = yadsl_graph_edge_exists_check(pGraph, buffer, buffer2, &actual);
		if (graphId == YADSL_GRAPH_RET_COND_OK && actual != expected)
			return TESTER_RETURN;
	} else if matches(command, "addedge") {
		char* edge;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_ARGUMENT;
		if ((edge = strdup(buffer3)) == NULL)
			return TESTER_MALLOC;
		if (graphId = yadsl_graph_edge_add(pGraph, buffer, buffer2, edge))
			free(edge);
	} else if matches(command, "getedge") {
		char* actual;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_ARGUMENT;
		graphId = yadsl_graph_edge_get(pGraph, buffer, buffer2, &actual);
		if (graphId == YADSL_GRAPH_RET_COND_OK) {
			if (nmatches(actual, buffer3))
				return TESTER_RETURN;
		}
	} else if matches(command, "removeedge") {
		if (TesterParseArguments("ss", buffer, buffer2) != 2)
			return TESTER_ARGUMENT;
		graphId = yadsl_graph_edge_remove(pGraph, buffer, buffer2);
	} else if matches(command, "setvertexflag") {
		int flag;
		if (TesterParseArguments("si", buffer, &flag) != 2)
			return TESTER_ARGUMENT;
		graphId = yadsl_graph_vertex_flag_set(pGraph, buffer, flag);
	} else if matches(command, "setallflags") {
		int flag;
		if (TesterParseArguments("i", &flag) != 1)
			return TESTER_ARGUMENT;
		graphId = yadsl_graph_vertex_flag_set_all(pGraph, flag);
	} else if matches(command, "getvertexflag") {
		int actual, expected;
		if (TesterParseArguments("si", buffer, &expected) != 2)
			return TESTER_ARGUMENT;
		graphId = yadsl_graph_vertex_flag_get(pGraph, buffer, &actual);
		if (graphId == YADSL_GRAPH_RET_COND_OK && actual != expected)
			return TESTER_RETURN;
	} else {
		return TESTER_COUNT;
	}
	return convertRet(graphId);
}

static TesterReturnValue parseGraphIoCommands(const char* command)
{
	GraphIoRet graphIoId = GRAPH_IO_OK;
	FILE* fp;
	if matches(command, "write") {
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		fp = fopen(buffer, "w");
		if (fp == NULL)
			return TESTER_FILE;
		graphIoId = graphWrite(pGraph, fp, _writeString, _writeString);
		fclose(fp);
	} else if matches(command, "read") {
		yadsl_GraphHandle* temp;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		fp = fopen(buffer, "r");
		if (fp == NULL)
			return TESTER_FILE;
		graphIoId = graphRead(&temp, fp, _readString, _readString,
			_cmpStrings, free, _cmpStrings, free);
		if (graphIoId == GRAPH_IO_OK) {
			yadsl_graph_destroy(pGraph);
			pGraph = temp;
		}
		fclose(fp);
	} else {
		return TESTER_COUNT;
	}
	return convertIoRet(graphIoId);
}

static void visitVertexCallback(void* vertex)
{
	printf("%s was visited\n", (char*) vertex);
}

static void visitEdgeCallback(void* source, void* edge, void* dest)
{
	printf("%s was visited (%s -> %s)\n",
		(char*) edge, (char*) source, (char*) dest);
}

static TesterReturnValue parseGraphSearchCommands(const char* command)
{
	GraphSearchRet graphSearchId = GRAPH_SEARCH_OK;
	int flag;
	if matches(command, "dfs") {
		if (TesterParseArguments("si", buffer, &flag) != 2)
			return TESTER_ARGUMENT;
		graphSearchId = graphDFS(pGraph,
			buffer,
			flag,
			visitVertexCallback,
			visitEdgeCallback);
	} else if matches(command, "bfs") {
		if (TesterParseArguments("si", buffer, &flag) != 2)
			return TESTER_ARGUMENT;
		graphSearchId = graphBFS(pGraph,
			buffer,
			flag,
			visitVertexCallback,
			visitEdgeCallback);
	} else {
		return TESTER_COUNT;
	}
	return convertSearchRet(graphSearchId);
}

// Here the TESTER_COUNT is being used as a flag to indicate that
// the command is not of a certain type, since it is never used truly for
// testing purposes.
TesterReturnValue TesterParseCallback(const char* command)
{
	TesterReturnValue ret;
	if ((ret = parseGraphCommands(command))
		!= TESTER_COUNT)
		return ret;
	if ((ret = parseGraphIoCommands(command))
		!= TESTER_COUNT)
		return ret;
	if ((ret = parseGraphSearchCommands(command))
		!= TESTER_COUNT)
		return ret;
	return TESTER_COMMAND;
}

TesterReturnValue TesterExitCallback()
{
	if (pGraph) yadsl_graph_destroy(pGraph);
#ifdef _DEBUG
	if (getGraphSearchNodeRefCount())
		return TESTER_MEMLEAK;
#endif
	return TESTER_OK;
}

static int _cmpStrings(void* a, void* b)
{
	return matches((char*) a, (char*) b);
}

static int _readString(FILE* fp, void** ppVertex)
{
	return ((*ppVertex = TesterUtilsDeserializeString(fp)) == NULL);
}

static int _writeString(FILE* fp, void* v)
{
	return TesterUtilsSerializeString(fp, (char*) v);
}

static TesterReturnValue convertRet(yadsl_GraphReturnCondition graphId)
{
	switch (graphId) {
	case YADSL_GRAPH_RET_COND_OK:
		return TESTER_OK;
	case YADSL_GRAPH_RET_COND_EMPTY:
		return TesterExternalReturnValue("empty");
	case YADSL_GRAPH_RET_COND_CONTAINS_VERTEX:
		return TesterExternalReturnValue("contains vertex");
	case YADSL_GRAPH_RET_COND_DOES_NOT_CONTAIN_VERTEX:
		return TesterExternalReturnValue("does not contain vertex");
	case YADSL_GRAPH_RET_COND_CONTAINS_EDGE:
		return TesterExternalReturnValue("contains edge");
	case YADSL_GRAPH_RET_COND_DOES_NOT_CONTAIN_EDGE:
		return TesterExternalReturnValue("does not contain edge");
	case YADSL_GRAPH_RET_COND_MEMORY:
		return TesterExternalReturnValue("memory");
	case YADSL_GRAPH_RET_COND_PARAMETER:
		return TesterExternalReturnValue("parameter");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

static TesterReturnValue convertIoRet(GraphIoRet graphIoId)
{
	switch (graphIoId) {
	case GRAPH_IO_OK:
		return TESTER_OK;
	case GRAPH_IO_MEMORY:
		return TesterExternalReturnValue("memory");
	case GRAPH_IO_WRITING_FAILURE:
		return TesterExternalReturnValue("writing failure");
	case GRAPH_IO_CREATION_FAILURE:
		return TesterExternalReturnValue("creation failure");
	case GRAPH_IO_SAME_CREATION:
		return TesterExternalReturnValue("same creation");
	case GRAPH_IO_FILE_ERROR:
		return TesterExternalReturnValue("file error");
	case GRAPH_IO_DEPRECATED_FILE_FORMAT:
		return TesterExternalReturnValue("deprecated file format");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

static TesterReturnValue convertSearchRet(GraphSearchRet graphSearchId)
{
	switch (graphSearchId) {
	case GRAPH_SEARCH_OK:
		return TESTER_OK;
	case GRAPH_SEARCH_DOES_NOT_CONTAIN_VERTEX:
		return TesterExternalReturnValue("does not contain vertex");
	case GRAPH_SEARCH_VERTEX_ALREADY_VISITED:
		return TesterExternalReturnValue("vertex already visited");
	case GRAPH_SEARCH_MEMORY:
		return TesterExternalReturnValue("memory");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

bool parseEdgeDirection(const char* buffer, yadsl_GraphEdgeDirection* edge_direction_ptr)
{
	if matches(buffer, "in") {
		*edge_direction_ptr = YADSL_GRAPH_EDGE_DIR_IN;
	} else if matches(buffer, "out") {
		*edge_direction_ptr = YADSL_GRAPH_EDGE_DIR_OUT;
	} else if matches(buffer, "both") {
		*edge_direction_ptr = YADSL_GRAPH_EDGE_DIR_BOTH;
	} else {
		TesterLog("Unknown edge direction \"%s\"", buffer);
		return false;
	}
	return true;
}

bool parseIterationDirection(const char* buffer, yadsl_GraphIterationDirection* iteration_direction_ptr)
{
	if matches(buffer, "next") {
		*iteration_direction_ptr = YADSL_GRAPH_ITER_DIR_NEXT;
	} else if matches(buffer, "previous") {
		*iteration_direction_ptr = YADSL_GRAPH_ITER_DIR_PREVIOUS;
	} else {
		TesterLog("Unknown iteration direction \"%s\"", buffer);
		return false;
	}
	return true;
}