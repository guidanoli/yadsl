#include <stdio.h>
#include <string.h>
#include "graph.h"

Graph *g = NULL;

char *test(GraphReturnID *pid, int *nid, unsigned long size, GraphEdgeType type,
           char **cmds, int count, const char *inFile, const char *outFile)
{
    unsigned long pSize, u, v;
    char cmd, flag[128], end = 0;
    int i, tokens;
    GraphEdgeType t;
    FILE *fp;
    if (inFile == NULL) {
        if (*pid = graphCreate(&g, size, type)) {
            if (*pid == GRAPH_RETURN_MEMORY)
                g = NULL;
            return "Could not create graph";
        }
    } else {
        fp = fopen(inFile, "r");
        printf("in = %s\n", inFile);
        if (fp == NULL)
            return "Could not open input file";
        if (*pid = graphRead(&g, fp)) {
            if (*pid == GRAPH_RETURN_MEMORY)
                g = NULL;
            return "Could not create graph from file";
        }
    }
    if ((*pid = graphGetNumberOfVertices(g, &pSize)) || pSize != size)
        return "Could not determine number of vertices or it is wrong";
    for (i = 0; i < count; ++i) {
        *nid = i+1;
        if (sscanf(cmds[i], "--%s", flag) == 1)
            continue;
        if ((tokens = sscanf(cmds[i], "%lu,%lu%c%c", &u, &v, &cmd, &end)) != 3
            && (tokens = sscanf(cmds[i], "%lu%c%c", &u, &cmd, &end)) != 2
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
                *pid = graphWrite(g, fp);
                fclose(fp);
                if (*pid)
                    return "Could not write to file";
                fprintf(stdout, "[%d] Wrote to file\n", *nid);
                break;
            default:
                return "Unknown command";
            }
        } else if (tokens == 2) {
            switch (cmd) {
            case 'i':
                switch (*pid = graphGetNextNeighbour(g, u, &v)) {
                case GRAPH_RETURN_OK:
                    fprintf(stdout, "[%d] %lu\n", *nid, v);
                    break;
                case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
                    fprintf(stdout, "[%d] Edge %lu has no neighbours\n", *nid, u);
                    break;
                default:
                    return "Could not get next neighbour";
                }
                break;
            case 'n':
                if (*pid = graphGetNumberOfNeighbours(g, u, &v))
                    return "Could not get number of neighbours";
                fprintf(stdout, "[%d] %lu\n", *nid, v);
                break;
            default:
                return "Unknown command";
            }
        } else if (tokens == 3) {
            switch (cmd) {
            case '+':
                if (*pid = graphAddEdge(g, u, v))
                    return "Could not add edge";
                break;
            case '-':
                if (*pid = graphRemoveEdge(g, u, v))
                    return "Could not remove edge";
                break;
            case '?':
                *pid = graphContainsEdge(g, u, v);
                switch(*pid) {
                    case GRAPH_RETURN_CONTAINS_EDGE:
                        fprintf(stdout, "[%d] Contains (%lu,%lu)\n", *nid, u, v);
                        break;
                    case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
                        fprintf(stdout, "[%d] Does not contain (%lu,%lu)\n", *nid, u, v);
                        break;
                    default:
                        return "Could not assert if edge was added or not";
                }
                break;
            default:
                return "Unknown command";
            }
        }

    }
    return NULL;
}

char *parse(int *nid, unsigned long *pSize, GraphEdgeType *pType,
            char **inputFilename, char **outputFilename, char **argv, int argc)
{
    int i;
    char flag[64], value[64], c = '0';
    *nid = 1;
    for (i = 0; i < argc; ++i) {
        if (sscanf(argv[i], "--%[^=]=%s", flag, value) == 2) {
            if (!strcmp(flag, "size")) {
                if (sscanf(value, "%lu%c", pSize, &c) != 1 || c != '0')
                    return "Could not set graph size";
            } else if (!strcmp(flag, "type")) {
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
                printf("This is an interactive module of the graph library\n");
                printf("You interact with one graph object at all times\n");
                printf("Actions to the graph are parsed by the command line arguments\n");
                printf("The registered actions are the following:\n");
                printf("n\tprints the number of vertices\n");
                printf("t\tprints the type of the graph\n");
                printf("Xi\tprints next neighbour of edge X\n");
                printf("Xn\tprints the number of neighbours of X\n");
                printf("X,Y+\tadds edge XY\n");
                printf("X,Y-\tremoves edge XY\n");
                printf("X,Y?\tchecks if edge XY exists\n");
                return NULL; /* abort parsing */
            } else {
                return "Unknown flag";
            }
        }
        *nid++;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    GraphReturnID id = 0;
    int nid = 1;
    unsigned long v = 10;
    GraphEdgeType type = GRAPH_EDGE_TYPE_UNDIRECTED;
    char *str = NULL, c = '0';
    char *inputFilename = NULL, *outputFilename = "out.graph";
    /* ignore program name */
    char **commands = argv + 1;
    int commandCount = argc - 1;
    /* parse arguments */
    str = parse(&nid, &v, &type, &inputFilename, &outputFilename, commands, commandCount);
    if (str) {
        fprintf(stderr, "Error while parsing argument #%d (%s): %s\n", nid, argv[nid], str);
        return 1;
    }
    /* run tests */
    str = test(&id, &nid, v, type, commands, commandCount, inputFilename, outputFilename);
    if (g)
        graphDestroy(g);
    g = NULL;
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
