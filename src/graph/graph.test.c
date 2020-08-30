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

const char* yadsl_tester_help_strings[] = {
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

static yadsl_TesterRet convertRet(yadsl_GraphRet graphId);
static yadsl_TesterRet convertIoRet(GraphIoRet graphIoId);
static yadsl_TesterRet convertSearchRet(GraphSearchRet graphSearchId);

yadsl_TesterRet yadsl_tester_init()
{
	pGraph = yadsl_graph_create(1, _cmpStrings, free, _cmpStrings, free);
	return pGraph ? YADSL_TESTER_RET_OK : YADSL_TESTER_RET_MALLOC;
}

static yadsl_TesterRet parseGraphCommands(const char* command)
{
	yadsl_GraphRet graphId = YADSL_GRAPH_RET_OK;
	if matches(command, "create") {
		int is_directed;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		is_directed = matches(buffer, "DIRECTED");
		yadsl_GraphHandle* temp = yadsl_graph_create(
			is_directed, _cmpStrings, free, _cmpStrings, free);
		if (temp) {
			yadsl_graph_destroy(pGraph);
			pGraph = temp;
		} else {
			return YADSL_TESTER_RET_MALLOC;
		}
	} else if matches(command, "isdirected") {
		bool actual, expected;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = matches(buffer, "YES");
		graphId = yadsl_graph_is_directed_check(pGraph, &actual);
		if (graphId == YADSL_GRAPH_RET_OK && expected != actual)
			return YADSL_TESTER_RET_RETURN;
	} else if matches(command, "vertexcount") {
		size_t actual, expected;
		if (yadsl_tester_parse_arguments("z", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		graphId = yadsl_graph_vertex_count_get(pGraph, &actual);
		if (graphId == YADSL_GRAPH_RET_OK && expected != actual)
			return YADSL_TESTER_RET_RETURN;
	} else if matches(command, "ivertices") {
		char* vertex;
		yadsl_GraphIterationDirection iteration_direction;
		if (yadsl_tester_parse_arguments("ss", buffer, buffer2) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		if (!parseIterationDirection(buffer, &iteration_direction))
			return YADSL_TESTER_RET_ARGUMENT;
		graphId = yadsl_graph_vertex_iter(pGraph, iteration_direction, &vertex);
		if (graphId == YADSL_GRAPH_RET_OK) {
			if (nmatches(buffer2, vertex))
				return YADSL_TESTER_RET_RETURN;
		}
	} else if matches(command, "degree") {
		size_t actual, expected;
		yadsl_GraphEdgeDirection edge_direction;
		if (yadsl_tester_parse_arguments("ssz", buffer, buffer2, &expected) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		if (!parseEdgeDirection(buffer2, &edge_direction))
			return YADSL_TESTER_RET_ARGUMENT;
		graphId = yadsl_graph_vertex_degree_get(pGraph, buffer, edge_direction, &actual);
		if (graphId == YADSL_GRAPH_RET_OK && expected != actual)
			return YADSL_TESTER_RET_RETURN;
	} else if matches(command, "ineighbours") {
		char* v, * uv;
		yadsl_GraphIterationDirection iteration_direction;
		yadsl_GraphEdgeDirection edge_direction;
		if (yadsl_tester_parse_arguments("sssss", buffer, buffer2, buffer3, buffer4, buffer5) != 5)
			return YADSL_TESTER_RET_ARGUMENT;
		if (!parseIterationDirection(buffer2, &iteration_direction))
			return YADSL_TESTER_RET_ARGUMENT;
		if (!parseEdgeDirection(buffer3, &edge_direction))
			return YADSL_TESTER_RET_ARGUMENT;
		graphId = yadsl_graph_vertex_nb_iter(pGraph, buffer, edge_direction, iteration_direction, &v, &uv);
		if (graphId == YADSL_GRAPH_RET_OK) {
			if (nmatches(v, buffer4) || nmatches(uv, buffer5))
				return YADSL_TESTER_RET_RETURN;
		}
	} else if matches(command, "containsvertex") {
		bool actual, expected;
		if (yadsl_tester_parse_arguments("ss", buffer, buffer2) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = TesterUtilsGetYesOrNoFromString(buffer2);
		graphId = yadsl_graph_vertex_exists_check(pGraph, buffer, &actual);
		if (graphId == YADSL_GRAPH_RET_OK && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if matches(command, "addvertex") {
		char* vertex;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((vertex = strdup(buffer)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if (graphId = yadsl_graph_vertex_add(pGraph, vertex))
			free(vertex);
	} else if matches(command, "removevertex") {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		graphId = yadsl_graph_vertex_remove(pGraph, buffer);
	} else if matches(command, "containsedge") {
		bool actual, expected;
		if (yadsl_tester_parse_arguments("sss", buffer, buffer2, buffer3) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = TesterUtilsGetYesOrNoFromString(buffer3);
		graphId = yadsl_graph_edge_exists_check(pGraph, buffer, buffer2, &actual);
		if (graphId == YADSL_GRAPH_RET_OK && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if matches(command, "addedge") {
		char* edge;
		if (yadsl_tester_parse_arguments("sss", buffer, buffer2, buffer3) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((edge = strdup(buffer3)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if (graphId = yadsl_graph_edge_add(pGraph, buffer, buffer2, edge))
			free(edge);
	} else if matches(command, "getedge") {
		char* actual;
		if (yadsl_tester_parse_arguments("sss", buffer, buffer2, buffer3) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		graphId = yadsl_graph_edge_get(pGraph, buffer, buffer2, &actual);
		if (graphId == YADSL_GRAPH_RET_OK) {
			if (nmatches(actual, buffer3))
				return YADSL_TESTER_RET_RETURN;
		}
	} else if matches(command, "removeedge") {
		if (yadsl_tester_parse_arguments("ss", buffer, buffer2) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		graphId = yadsl_graph_edge_remove(pGraph, buffer, buffer2);
	} else if matches(command, "setvertexflag") {
		int flag;
		if (yadsl_tester_parse_arguments("si", buffer, &flag) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		graphId = yadsl_graph_vertex_flag_set(pGraph, buffer, flag);
	} else if matches(command, "setallflags") {
		int flag;
		if (yadsl_tester_parse_arguments("i", &flag) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		graphId = yadsl_graph_vertex_flag_set_all(pGraph, flag);
	} else if matches(command, "getvertexflag") {
		int actual, expected;
		if (yadsl_tester_parse_arguments("si", buffer, &expected) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		graphId = yadsl_graph_vertex_flag_get(pGraph, buffer, &actual);
		if (graphId == YADSL_GRAPH_RET_OK && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return convertRet(graphId);
}

static yadsl_TesterRet parseGraphIoCommands(const char* command)
{
	GraphIoRet graphIoId = GRAPH_IO_OK;
	FILE* fp;
	if matches(command, "write") {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		fp = fopen(buffer, "w");
		if (fp == NULL)
			return YADSL_TESTER_RET_FILE;
		graphIoId = graphWrite(pGraph, fp, _writeString, _writeString);
		fclose(fp);
	} else if matches(command, "read") {
		yadsl_GraphHandle* temp;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		fp = fopen(buffer, "r");
		if (fp == NULL)
			return YADSL_TESTER_RET_FILE;
		graphIoId = graphRead(&temp, fp, _readString, _readString,
			_cmpStrings, free, _cmpStrings, free);
		if (graphIoId == GRAPH_IO_OK) {
			yadsl_graph_destroy(pGraph);
			pGraph = temp;
		}
		fclose(fp);
	} else {
		return YADSL_TESTER_RET_COUNT;
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

static yadsl_TesterRet parseGraphSearchCommands(const char* command)
{
	GraphSearchRet graphSearchId = GRAPH_SEARCH_OK;
	int flag;
	if matches(command, "dfs") {
		if (yadsl_tester_parse_arguments("si", buffer, &flag) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		graphSearchId = graphDFS(pGraph,
			buffer,
			flag,
			visitVertexCallback,
			visitEdgeCallback);
	} else if matches(command, "bfs") {
		if (yadsl_tester_parse_arguments("si", buffer, &flag) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		graphSearchId = graphBFS(pGraph,
			buffer,
			flag,
			visitVertexCallback,
			visitEdgeCallback);
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return convertSearchRet(graphSearchId);
}

// Here the YADSL_TESTER_RET_COUNT is being used as a flag to indicate that
// the command is not of a certain type, since it is never used truly for
// testing purposes.
yadsl_TesterRet yadsl_tester_parse(const char* command)
{
	yadsl_TesterRet ret;
	if ((ret = parseGraphCommands(command))
		!= YADSL_TESTER_RET_COUNT)
		return ret;
	if ((ret = parseGraphIoCommands(command))
		!= YADSL_TESTER_RET_COUNT)
		return ret;
	if ((ret = parseGraphSearchCommands(command))
		!= YADSL_TESTER_RET_COUNT)
		return ret;
	return YADSL_TESTER_RET_COMMAND;
}

yadsl_TesterRet yadsl_tester_release()
{
	if (pGraph) yadsl_graph_destroy(pGraph);
#ifdef _DEBUG
	if (getGraphSearchNodeRefCount())
		return YADSL_TESTER_RET_MEMLEAK;
#endif
	return YADSL_TESTER_RET_OK;
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

static yadsl_TesterRet convertRet(yadsl_GraphRet graphId)
{
	switch (graphId) {
	case YADSL_GRAPH_RET_OK:
		return YADSL_TESTER_RET_OK;
	case YADSL_GRAPH_RET_EMPTY:
		return yadsl_tester_return_external_value("empty");
	case YADSL_GRAPH_RET_CONTAINS_VERTEX:
		return yadsl_tester_return_external_value("contains vertex");
	case YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX:
		return yadsl_tester_return_external_value("does not contain vertex");
	case YADSL_GRAPH_RET_CONTAINS_EDGE:
		return yadsl_tester_return_external_value("contains edge");
	case YADSL_GRAPH_RET_DOES_NOT_CONTAIN_EDGE:
		return yadsl_tester_return_external_value("does not contain edge");
	case YADSL_GRAPH_RET_MEMORY:
		return yadsl_tester_return_external_value("memory");
	case YADSL_GRAPH_RET_PARAMETER:
		return yadsl_tester_return_external_value("parameter");
	default:
		return yadsl_tester_return_external_value("unknown");
	}
}

static yadsl_TesterRet convertIoRet(GraphIoRet graphIoId)
{
	switch (graphIoId) {
	case GRAPH_IO_OK:
		return YADSL_TESTER_RET_OK;
	case GRAPH_IO_MEMORY:
		return yadsl_tester_return_external_value("memory");
	case GRAPH_IO_WRITING_FAILURE:
		return yadsl_tester_return_external_value("writing failure");
	case GRAPH_IO_CREATION_FAILURE:
		return yadsl_tester_return_external_value("creation failure");
	case GRAPH_IO_SAME_CREATION:
		return yadsl_tester_return_external_value("same creation");
	case GRAPH_IO_FILE_ERROR:
		return yadsl_tester_return_external_value("file error");
	case GRAPH_IO_DEPRECATED_FILE_FORMAT:
		return yadsl_tester_return_external_value("deprecated file format");
	default:
		return yadsl_tester_return_external_value("unknown");
	}
}

static yadsl_TesterRet convertSearchRet(GraphSearchRet graphSearchId)
{
	switch (graphSearchId) {
	case GRAPH_SEARCH_OK:
		return YADSL_TESTER_RET_OK;
	case GRAPH_SEARCH_DOES_NOT_CONTAIN_VERTEX:
		return yadsl_tester_return_external_value("does not contain vertex");
	case GRAPH_SEARCH_VERTEX_ALREADY_VISITED:
		return yadsl_tester_return_external_value("vertex already visited");
	case GRAPH_SEARCH_MEMORY:
		return yadsl_tester_return_external_value("memory");
	default:
		return yadsl_tester_return_external_value("unknown");
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
		yadsl_tester_log("Unknown edge direction \"%s\"", buffer);
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
		yadsl_tester_log("Unknown iteration direction \"%s\"", buffer);
		return false;
	}
	return true;
}