#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "graph.h"
#include "graphio.h"
#include "var.h"

/* Help */

static const char *helpStrings[] = {
    "This is an interactive module of the graph library",
    "You will interact with the same graph object at all times",
    "Actions and flags to the graph are parsed by the command line arguments",
    "",
    "The registered actions are the following:",
    "n\tprints the number of vertices",
    "i\tprints next vertex",
    "t\tprints the type of the graph",
    "w\twrites the graph to the output file",
    "Xi\tprints next neighbour of edge X",
    "Xn\tprints the number of neighbours of X",
    "X+\tadds vertex X",
    "X-\tremoves vertex X",
    "X?\tchecks if vertex X exists",
    "X,Y+\tadds edge XY",
    "X,Y-\tremoves edge XY",
    "X,Y?\tchecks if edge XY exists",
    "",
    "The registered flags are the following:",
    "type\tsets the graph type (DIRECTED or UNDIRECTED)",
    "in\tsets the input graph file",
    "out\tsets the output graph file",
    NULL,
};

/* Graph */

static Graph *g = NULL;
Variable *v1 = NULL, *v2 = NULL;

/* Private functions prototypes */

static void print_help();
static int _cmpVariables(void *a, void *b);
static void _freeVariables(void *v);
static int _readVariables(FILE *fp, void **ppVertex);
static int _writeVariables(FILE *fp, void *v);
static void _garbageCollectVariables();
#define _transferOwnership(...) __transferOwnership(__VA_ARGS__, NULL);
static void __transferOwnership(Variable **v, ...);
static char *parse(int *nid, GraphEdgeType *pType, char **inputFilename,
    char **outputFilename, char **argv, int argc);
static char *test(GraphReturnID *pid, int *nid, GraphEdgeType type,
    char **cmds, int count, const char *inFile, const char *outFile);

/* Main function */

int main(int argc, char **argv)
{
    GraphReturnID id = GRAPH_RETURN_OK;
    int nid = 1;
    GraphEdgeType type = GRAPH_EDGE_TYPE_UNDIRECTED;
    char *str = NULL;
    char *inputFilename = NULL, *outputFilename = "out.graph";
    /* ignore program name */
    char **commands = argv + 1;
    int commandCount = argc - 1;
    /* parse arguments */
    if (commandCount > 0) {
        str = parse(&nid, &type, &inputFilename, &outputFilename, commands, commandCount);
        if (str) {
            fprintf(stderr, "Error while parsing argument #%d (%s): %s\n", nid, argv[nid], str);
            return 1;
        }
    } else {
        print_help();
        return 0;
    }
    /* run tests */
    str = test(&id, &nid, type, commands, commandCount, inputFilename, outputFilename);
    _garbageCollectVariables();
    if (g)
        graphDestroy(g);
    g = NULL;
#ifdef _DEBUG
    if (varGetRefCount())
        fprintf(stderr, "Memory leak detected\n");
#endif
    if (str) {
        if (id) {
            fprintf(stderr, "Error #%d on the action #%d (%s): %s\n", id, nid, argv[nid], str);
        } else {
            fprintf(stderr, "Error on the action #%d (%s): %s\n", nid, argv[nid], str);
        }
        return 1;
    } else {
        fprintf(stdout, "No errors.\n");
    }
    return 0;
}

/* Private functions implementations */
static char *test(GraphReturnID *pid, int *nid, GraphEdgeType type,
    char **cmds, int count, const char *inFile, const char *outFile)
{
    unsigned long pSize;
    char cmd, flag[128], buff1[1024], buff2[1024], end = 0;
    int i, tokens, ret;
    GraphEdgeType t;
    FILE *fp;
    if (inFile == NULL) {
        if (*pid = graphCreate(&g, type, _cmpVariables, _freeVariables)) {
            if (*pid == GRAPH_RETURN_MEMORY)
                g = NULL;
            return "Could not create graph";
        }
    } else {
        fp = fopen(inFile, "r");
        if (fp == NULL)
            return "Could not open input file";
        if (*pid = graphRead(&g, fp, _readVariables, _cmpVariables, _freeVariables)) {
            if (*pid == GRAPH_RETURN_MEMORY)
                g = NULL;
            return "Could not create graph from file";
        }
    }
    for (i = 0; i < count; ++i) {
        *nid = i + 1;
        if (sscanf(cmds[i], "--%s", flag) == 1)
            continue;
        if ((tokens = sscanf(cmds[i], "%[^,],%[^+?-]%c%c", buff1, buff2, &cmd, &end)) != 3
            && (tokens = sscanf(cmds[i], "%[^in+?-]%c%c", buff1, &cmd, &end)) != 2
            && (tokens = sscanf(cmds[i], "%c%c", &cmd, &end)) != 1)
            return "Parsing error";
        if (tokens == 1) {
            switch (cmd) {
            case 't':
                if (*pid = graphGetType(g, &t))
                    return "Could not get type";
                switch (t) {
                case GRAPH_EDGE_TYPE_DIRECTED:
                    fprintf(stdout, "[%d] Directed\n", *nid);
                    break;
                case GRAPH_EDGE_TYPE_UNDIRECTED:
                    fprintf(stdout, "[%d] Undirected\n", *nid);
                    break;
                default:
                    return "Unknown graph type";
                }
                break;
            case 'n':
                if (*pid = graphGetNumberOfVertices(g, &pSize))
                    return "Could not get number of vertices";
                fprintf(stdout, "[%d] %lu\n", *nid, pSize);
                break;
            case 'w':
                fp = fopen(outFile, "w");
                if (!fp)
                    return "Could not open file";
                *pid = graphWrite(g, fp, _writeVariables);
                fclose(fp);
                if (*pid)
                    return "Could not write to file";
                fprintf(stdout, "[%d] Wrote to file\n", *nid);
                break;
            case 'i':
                if (*pid = graphGetNextVertex(g, &v1))
                    return "Could not get next vertex";
                fprintf(stdout, "[%d] ", *nid);
                varWrite(v1, stdout);
                fprintf(stdout, "\n");
                _transferOwnership(&v1);
                break;
            default:
                return "Unknown command";
            }
        } else if (tokens == 2) {
            if (varCreate(buff1, &v1))
                return "Could not create variable";
            switch (cmd) {
            case 'i':
                switch (*pid = graphGetNextNeighbour(g, v1, &v2)) {
                case GRAPH_RETURN_OK:
                    fprintf(stdout, "[%d] ", *nid);
                    ret = varWrite(v2, stdout);
                    _transferOwnership(&v2);
                    if (ret)
                        return "Could not write variable";
                    fprintf(stdout, "\n");
                    break;
                case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
                    fprintf(stdout, "[%d] Edge ", *nid);
                    if (varWrite(v1, stdout))
                        return "Could not write variable";
                    fprintf(stdout, " has no neighbours\n");
                    break;
                default:
                    return "Could not get next neighbour";
                }
                break;
            case 'n':
                if (*pid = graphGetNumberOfNeighbours(g, v1, &pSize))
                    return "Could not get number of neighbours";
                fprintf(stdout, "[%d] %lu\n", *nid, pSize);
                break;
            case '?':
                *pid = graphContainsVertex(g, v1);
                switch (*pid) {
                case GRAPH_RETURN_CONTAINS_VERTEX:
                    fprintf(stdout, "[%d] Contains ", *nid);
                    varWrite(v1, stdout);
                    fprintf(stdout, "\n");
                    break;
                case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
                    fprintf(stdout, "[%d] Does not contain ", *nid);
                    varWrite(v1, stdout);
                    fprintf(stdout, "\n");
                    break;
                default:
                    return "Could not assert if vertex was added or not";
                }
                break;
            case '+':
                if (*pid = graphAddVertex(g, v1))
                    return "Could not add vertex";
                _transferOwnership(&v1);
                break;
            case '-':
                if (*pid = graphRemoveVertex(g, v1))
                    return "Could not remove vertex";
                break;
            default:
                return "Unknown command";
            }
        } else if (tokens == 3) {
            if (varCreate(buff1, &v1))
                return "Could not create first variable";
            if (varCreate(buff2, &v2))
                return "Could not create second variable";
            switch (cmd) {
            case '+':
                if (*pid = graphAddEdge(g, v1, v2))
                    return "Could not add edge";
                break;
            case '-':
                if (*pid = graphRemoveEdge(g, v1, v2))
                    return "Could not remove edge";
                break;
            case '?':
                *pid = graphContainsEdge(g, v1, v2);
                switch (*pid) {
                case GRAPH_RETURN_CONTAINS_EDGE:
                    fprintf(stdout, "[%d] Contains ", *nid);
                    break;
                case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
                    fprintf(stdout, "[%d] Does not contain ", *nid);
                    break;
                default:
                    return "Could not assert if edge was added or not";
                }
                fprintf(stdout, "<");
                varWrite(v1, stdout);
                fprintf(stdout, ", ");
                varWrite(v2, stdout);
                fprintf(stdout, ">\n");
                break;
            default:
                return "Unknown command";
            }
        }
        /* Local variables will be deallocated */
        _garbageCollectVariables();
    }
    return NULL;
}

static void print_help() {
    const char **str = helpStrings;
    for (; *str; ++str) puts(*str);
}

static char *parse(int *nid, GraphEdgeType *pType, char **inputFilename,
    char **outputFilename, char **argv, int argc)
{
    int i;
    char flag[64], value[64], c = '0';
    *nid = 1;
    for (i = 0; i < argc; ++i) {
        if (sscanf(argv[i], "--%[^=]=%s", flag, value) == 2) {
            if (!strcmp(flag, "type")) {
                if (!strcmp(value, "DIRECTED"))
                    *pType = GRAPH_EDGE_TYPE_DIRECTED;
                else if (!strcmp(value, "UNDIRECTED"))
                    *pType = GRAPH_EDGE_TYPE_UNDIRECTED;
                else
                    return "Could not set graph type";
            } else if (!strcmp(flag, "in")) {
                *inputFilename = strdup(value);
            } else if (!strcmp(flag, "out")) {
                *outputFilename = strdup(value);
            } else {
                return "Unknown flag";
            }
        } else if (sscanf(argv[i], "--%s", flag) == 1) {
            if (!strcmp(flag, "help")) {
                print_help();
                return NULL; /* abort parsing */
            } else {
                return "Unknown flag";
            }
        }
        *nid++;
    }
    return NULL;
}

static void _freeVariables(void *v)
{
    varDestroy(v);
}

static int _cmpVariables(void *a, void *b)
{
    int cmpResult;
    if (varCompare(a, b, &cmpResult)) return 0;
    return cmpResult;
}

static int _readVariables(FILE *fp, void **ppVertex)
{
    return varSerializeRead(ppVertex, fp);
}

static int _writeVariables(FILE *fp, void *v)
{
    return varSerializeWrite(v, fp);
}

static void _garbageCollectVariables()
{
    if (v1) varDestroy(v1);
    if (v2) varDestroy(v2);
    v1 = NULL;
    v2 = NULL;
}

static void __transferOwnership(Variable **v, ...)
{
    va_list va;
    va_start(va, v);
    do { *v = NULL; }
    while (v = va_arg(va, Variable **));
    va_end(va);
}