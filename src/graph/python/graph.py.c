#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "pydefines.h"

#include "graph.h"

typedef struct {
    PyObject_HEAD
    Graph *ob_graph;
} GraphObject;

static void
decRefCallback(void *item)
{
	Py_DECREF((PyObject *) item);
}

static int
cmpCallback(void *a, void *b)
{
	int cmp;
	Py_XINCREF(a);
	Py_XINCREF(b);
	cmp = PyObject_RichCompareBool(
		(PyObject *) a,
		(PyObject *) b,
		Py_EQ);
	Py_XDECREF(a);
	Py_XDECREF(b);
	return cmp == -1 ? 0 : cmp;
}

static PyObject *
Graph_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
	GraphObject *self;
	self = (GraphObject *) type->tp_alloc(type, 0);
	if (self != NULL)
		self->ob_graph = NULL;
	return (PyObject *) self;
}

PyDoc_STRVAR(_Graph_init__doc__,
"Graph(/, directed=True)\n"
"--\n"
"\n"
"Python graph data structure.");

static int
Graph_init(GraphObject *self, PyObject *args, PyObject *kw)
{
	int isDirected = 1;
	static char *keywords[] = { "directed", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kw,
		"|p:pygraph.Graph.__init__", keywords, &isDirected))
		return -1;
	if (graphCreate(&self->ob_graph, isDirected,
		cmpCallback, decRefCallback,
		cmpCallback, decRefCallback))
		return -1;
	return 0;
}

static void
Graph_dealloc(GraphObject *self)
{
	if (self->ob_graph)
		graphDestroy(self->ob_graph);
	Py_TYPE(self)->tp_free((PyObject *) self);
}

PyDoc_STRVAR(_Graph_is_directed__doc__,
"is_directed(self, /) -> bool\n"
"--\n"
"\n"
"Check whether graph is directed or not");

static PyObject *
Graph_is_directed(GraphObject *self, PyObject *Py_UNUSED(ignored))
{
	int isDirected;
	if (graphIsDirected(self->ob_graph, &isDirected))
		return NULL;
	return PyBool_FromLong(isDirected);
}

PyDoc_STRVAR(_Graph_contains_vertex__doc__,
"contains_vertex(self, u : Object, /) -> None\n"
"--\n"
"\n"
"Check whether vertex is contained in graph");

static PyObject *
Graph_contains_vertex(GraphObject *self, PyObject *obj)
{
	int contains;
	if (graphContainsVertex(self->ob_graph, obj, &contains))
		PyErr_BadInternalCall();
	if (PyErr_Occurred())
		return NULL;
	return PyBool_FromLong(contains);
}

PyDoc_STRVAR(_Graph_add_vertex__doc__,
"add_vertex(self, u : Object, /) -> None\n"
"--\n"
"\n"
"Add vertex to graph");

static PyObject *
Graph_add_vertex(GraphObject *self, PyObject *obj)
{
	GraphRet returnId;
	returnId = graphAddVertex(self->ob_graph, obj);
	if (!returnId)
		Py_INCREF(obj);
	if (PyErr_Occurred())
		goto exit;
	switch (returnId) {
	case GRAPH_OK:
		Py_RETURN_NONE;
	case GRAPH_CONTAINS_VERTEX:
		PyErr_SetString(PyExc_RuntimeError,
			"Vertex already existed in graph");
		break;
	case GRAPH_MEMORY:
		return PyErr_NoMemory();
	default:
		Py_UNREACHABLE();
	}
exit:
	return NULL;
}

PyDoc_STRVAR(_Graph_remove_vertex__doc__,
"remove_vertex(self, u : Object, /) -> None\n"
"--\n"
"\n"
"Remove vertex from graph");

static PyObject *
Graph_remove_vertex(GraphObject *self, PyObject *obj)
{
	GraphRet returnId;
	returnId = graphRemoveVertex(self->ob_graph, obj);
	if (PyErr_Occurred())
		goto exit;
	switch (returnId) {
	case GRAPH_OK:
		Py_RETURN_NONE;
	case GRAPH_DOES_NOT_CONTAIN_VERTEX:
		PyErr_SetString(PyExc_RuntimeError,
			"Vertex not found in graph");
		break;
	default:
		Py_UNREACHABLE();
	}
exit:
	return NULL;
}

PyDoc_STRVAR(_Graph_get_vertex_count__doc__,
"get_vertex_count(/) -> int\n"
"--\n"
"\n"
"Get number of vertices in graph");

static PyObject *
Graph_get_vertex_count(GraphObject *self, PyObject *Py_UNUSED(ignored))
{
	size_t size;
	if (graphGetNumberOfVertices(self->ob_graph, &size))
		PyErr_BadInternalCall();
	return PyLong_FromSize_t(size);
}

PyDoc_STRVAR(_Graph_contains_edge__doc__,
"contains_edge(self, u : Object, v : Object, /) -> bool\n"
"--\n"
"\n"
"Check whether there is an edge from u to v in graph");

static PyObject *
Graph_contains_edge(GraphObject *self, PyObject *args)
{
	GraphRet returnId;
	PyObject *u, *v;
	int contains;
	if (!PyArg_ParseTuple(args,
		"OO:pygraph.Graph.contains_edge",
		&u, &v))
		goto exit;
	returnId = graphContainsEdge(self->ob_graph, u, v, &contains);
	if (PyErr_Occurred())
		goto exit;
	switch (returnId) {
	case GRAPH_OK:
		return PyBool_FromLong(contains);
	case GRAPH_DOES_NOT_CONTAIN_VERTEX:
		PyErr_SetString(PyExc_RuntimeError,
			"Vertex not found in graph");
		break;
	default:
		Py_UNREACHABLE();
	}
exit:
	return NULL;
}


PyDoc_STRVAR(_Graph_add_edge__doc__,
"add_edge(self, u : Object, v : Object, uv : Object, /) -> None\n"
"--\n"
"\n"
"Add edge uv (from u to v) to graph");

static PyObject *
Graph_add_edge(GraphObject *self, PyObject *args)
{
	GraphRet returnId;
	PyObject *u, *v, *uv;
	if (!PyArg_ParseTuple(args,
		"OOO:pygraph.Graph.add_edge",
		&u, &v, &uv))
		goto exit;
	returnId = graphAddEdge(self->ob_graph, u, v, uv);
	if (!returnId)
		Py_INCREF(uv);
	if (PyErr_Occurred())
		goto exit;
	switch (returnId) {
	case GRAPH_OK:
		Py_RETURN_NONE;
	case GRAPH_DOES_NOT_CONTAIN_VERTEX:
		PyErr_SetString(PyExc_RuntimeError,
			"Vertex not found in graph");
		break;
	case GRAPH_CONTAINS_EDGE:
		PyErr_SetString(PyExc_RuntimeError,
			"Edge already found in graph");
		break;
	case GRAPH_MEMORY:
		return PyErr_NoMemory();
	default:
		Py_UNREACHABLE();
	}
exit:
	return NULL;
}

PyDoc_STRVAR(_Graph_remove_edge__doc__,
"remove_edge(self, u : Object, v : Object, /) -> None\n"
"--\n"
"\n"
"Remove edge from u to v from graph");

static PyObject*
Graph_remove_edge(GraphObject* self, PyObject* args)
{
	GraphRet returnId;
	PyObject *u, *v;
	if (!PyArg_ParseTuple(args,
		"OO:pygraph.Graph.remove_edge",
		&u, &v))
		goto exit;
	returnId = graphRemoveEdge(self->ob_graph, u, v);
	if (PyErr_Occurred())
		goto exit;
	switch (returnId) {
	case GRAPH_OK:
		Py_RETURN_NONE;
	case GRAPH_DOES_NOT_CONTAIN_VERTEX:
		PyErr_SetString(PyExc_RuntimeError,
			"Vertex not found in graph");
		break;
	case GRAPH_DOES_NOT_CONTAIN_EDGE:
		PyErr_SetString(PyExc_RuntimeError,
			"Edge not found in graph");
		break;
	}
exit:
	return NULL;
}

PyDoc_STRVAR(_Graph_get_edge__doc__,
"get_edge(self, u : Object, v : Object, /) -> Object\n"
"--\n"
"\n"
"Get edge that goes from u to v in graph");

static PyObject*
Graph_get_edge(GraphObject* self, PyObject* args)
{
	GraphRet returnId;
	PyObject *u, *v, *uv;
	if (!PyArg_ParseTuple(args,
		"OO:pygraph.Graph.remove_edge",
		&u, &v))
		goto exit;
	returnId = graphGetEdge(self->ob_graph, u, v, &uv);
	if (PyErr_Occurred())
		goto exit;
	switch (returnId) {
	case GRAPH_OK:
		Py_INCREF(uv);
		return uv;
	case GRAPH_DOES_NOT_CONTAIN_VERTEX:
		PyErr_SetString(PyExc_RuntimeError,
			"Vertex not found in graph");
		break;
	case GRAPH_DOES_NOT_CONTAIN_EDGE:
		PyErr_SetString(PyExc_RuntimeError,
			"Edge not found in graph");
		break;
	}
exit:
	return NULL;
}

PyDoc_STRVAR(_Graph_vertices__doc__,
"vertices(self, /) -> tuple of Objects\n"
"--\n"
"\n"
"Get tuple of vertices in graph");

static PyObject *
Graph_vertices(GraphObject *self, PyObject *Py_UNUSED(ignored))
{
	size_t size;
	PyObject *tuple = NULL, *vertex;
	if (graphGetNumberOfVertices(self->ob_graph, &size))
		goto exit;
	tuple = PyTuple_New(size);
	if (tuple == NULL)
		return NULL;
	while (size--) {
		if (graphGetPreviousVertex(self->ob_graph, &vertex)) {
			Py_DECREF(tuple);
			goto exit;
		}
		Py_INCREF(vertex);
		PyTuple_SET_ITEM(tuple, size, vertex);
	}
	return tuple;
exit:
	PyErr_BadInternalCall();
	return NULL;
}

PyDoc_STRVAR(_Graph_neighbours__doc__,
"neighbours(self, v : Object, /, ingoing = True, outgoing = True)"
" -> tuple of tuples (neighbour, edge)\n"
"--\n"
"\n"
"Get tuple for neighbours and edges of vertex v in graph\n"
"You can filter the neighbours by the edges connecting to v\n"
"  - ingoing stands for neighbours that have edges pointing to v\n"
"  - outgoing stands for neighbours that have edges coming from v\n");

static PyObject *
Graph_neighbours(GraphObject *self, PyObject *args, PyObject *kw)
{
	int in = 1, out = 1;
	size_t size;
	GraphRet returnId;
	PyObject *vertex, *tuple = NULL, *subtuple, *neighbour, *edge;
	GraphRet (*getDegree)(Graph *, void *, size_t *);
	GraphRet (*getPrevious)(Graph *, void *, void **, void **);
	static char *keywords[] = { "vertex", "ingoing", "outgoing", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kw,
		"O|pp:pygraph.Graph.neighbours", keywords, &vertex, &in, &out))
		return NULL;
	if (in && out) {
		getDegree = graphGetVertexDegree;
		getPrevious = graphGetPreviousNeighbour;
	} else if (in) {
		getDegree = graphGetVertexInDegree;
		getPrevious = graphGetPreviousInNeighbour;
	} else if (out) {
		getDegree = graphGetVertexOutDegree;
		getPrevious = graphGetPreviousOutNeighbour;
	} else {
		PyErr_SetString(PyExc_KeyError,
			"Keys ingoing and outgoing cannot be both False");
		goto exit;
	}
	if (returnId = getDegree(self->ob_graph, vertex, &size)) {
		switch (returnId) {
		case GRAPH_DOES_NOT_CONTAIN_VERTEX:
			PyErr_SetString(PyExc_RuntimeError,
				"Vertex not found in graph");
			goto exit;
		default:
			Py_UNREACHABLE();
		}
	}
	if (PyErr_Occurred())
		goto exit;
	if ((tuple = PyTuple_New(size)) == NULL)
		goto exit;
	while (size--) {
		if (getPrevious(self->ob_graph, vertex, &neighbour, &edge))
			goto badinternalcall_exit;
		if (PyErr_Occurred())
			goto exit;
		if ((subtuple = PyTuple_Pack(2, neighbour, edge)) == NULL)
			goto exit;
		PyTuple_SET_ITEM(tuple, size, subtuple);
	}
	return tuple;
badinternalcall_exit:
	PyErr_BadInternalCall();
exit:
	Py_XDECREF(tuple);
	return NULL;
}

PyDoc_STRVAR(_Graph_degree__doc__,
"degree(self, v : Object, /, ingoing = True, outgoing = True)"
" -> degree of vertex\n"
"--\n"
"\n"
"Get degree (number of edges) of vertex v in graph\n"
"You can filter the degree by the edges connecting to v\n"
"  - ingoing stands for neighbours that have edges pointing to v\n"
"  - outgoing stands for neighbours that have edges coming from v\n");

static PyObject *
Graph_degree(GraphObject *self, PyObject *args, PyObject *kw)
{
	int in = 1, out = 1;
	size_t size;
	PyObject *vertex;
	GraphRet returnId;
	GraphRet(*getDegree)(Graph *, void *, size_t *);
	static char *keywords[] = { "vertex", "ingoing", "outgoing", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kw,
		"O|pp:pygraph.Graph.degree", keywords, &vertex, &in, &out))
		return NULL;
	if (in && out) {
		getDegree = graphGetVertexDegree;
	} else if (in) {
		getDegree = graphGetVertexInDegree;
	} else if (out) {
		getDegree = graphGetVertexOutDegree;
	} else {
		PyErr_SetString(PyExc_KeyError,
			"Keys ingoing and outgoing cannot be both False");
		goto exit;
	}
	if (returnId = getDegree(self->ob_graph, vertex, &size)) {
		switch (returnId) {
		case GRAPH_DOES_NOT_CONTAIN_VERTEX:
			PyErr_SetString(PyExc_RuntimeError,
				"Vertex not found in graph");
			goto exit;
		default:
			Py_UNREACHABLE();
		}
	}
	return PyLong_FromSize_t(size);
exit:
	return NULL;
}

PyDoc_STRVAR(_Graph_get_flag__doc__,
"get_flag(self, v : Object, /) -> int\n"
"--\n"
"\n"
"Get flag assigned to vertex");

static PyObject *
Graph_get_flag(GraphObject *self, PyObject *obj)
{
	int flag;
	switch (graphGetVertexFlag(self->ob_graph, obj, &flag)) {
	case GRAPH_OK:
		return PyLong_FromLong(flag);
	case GRAPH_DOES_NOT_CONTAIN_VERTEX:
		PyErr_SetString(PyExc_RuntimeError,
			"Vertex not found in graph");
		break;
	default:
		Py_UNREACHABLE();
	}
	return NULL;
}

PyDoc_STRVAR(_Graph_set_flag__doc__,
"set_flag(self, v : Object, f : int, /) -> None\n"
"--\n"
"\n"
"Assign flag to vertex");

static PyObject *
Graph_set_flag(GraphObject *self, PyObject *args)
{
	int flag;
	PyObject *vertex;
	if (!PyArg_ParseTuple(args, "Oi:pygraph.Graph.set_flag", &vertex, &flag))
		return NULL;
	switch (graphSetVertexFlag(self->ob_graph, vertex, flag)) {
	case GRAPH_OK:
		Py_RETURN_NONE;
	case GRAPH_DOES_NOT_CONTAIN_VERTEX:
		PyErr_SetString(PyExc_RuntimeError,
			"Vertex not found in graph");
		break;
	default:
		Py_UNREACHABLE();
	}
	return NULL;
}

PyDoc_STRVAR(_Graph_set_all_flags__doc__,
"set_all_flags(self, f : int, /) -> None\n"
"--\n"
"\n"
"Assign flag to all vertices");

static PyObject *
Graph_set_all_flags(GraphObject *self, PyObject *obj)
{
	int flag;
	if (PyLong_Check(obj)) {
		flag = PyLong_AsLong(obj);
		if (flag == -1 && PyErr_Occurred())
			return NULL;
		if (graphSetAllVerticesFlags(self->ob_graph, flag))
			PyErr_BadInternalCall();
		Py_RETURN_NONE;
	} else {
		PyErr_SetString(PyExc_TypeError, "Expected an int as first argument "
			"to set_all_flags");
		return NULL;
	}
}

PyMethodDef Graph_methods[] = {
	//
	// Metadata about graph
	//
	{
		"is_directed",
		(PyCFunction) Graph_is_directed,
		METH_NOARGS,
		_Graph_is_directed__doc__
	},
	//
	// Vertex data
	//
	{
		"vertices",
		(PyCFunction) Graph_vertices,
		METH_NOARGS,
		_Graph_vertices__doc__
	},
	{
		"contains_vertex",
		(PyCFunction) Graph_contains_vertex,
		METH_O,
		_Graph_contains_vertex__doc__
	},
	{
		"add_vertex",
		(PyCFunction) Graph_add_vertex,
		METH_O,
		_Graph_add_vertex__doc__
	},
	{
		"remove_vertex",
		(PyCFunction) Graph_remove_vertex,
		METH_O,
		_Graph_remove_vertex__doc__
	},
	{
		"get_vertex_count",
		(PyCFunction) Graph_get_vertex_count,
		METH_NOARGS,
		_Graph_get_vertex_count__doc__
	},
	//
	// Edge data
	//
	{
		"contains_edge",
		(PyCFunction) Graph_contains_edge,
		METH_VARARGS,
		_Graph_contains_edge__doc__
	},
	{
		"add_edge",
		(PyCFunction) Graph_add_edge,
		METH_VARARGS,
		_Graph_add_edge__doc__
	},
	{
		"remove_edge",
		(PyCFunction) Graph_remove_edge,
		METH_VARARGS,
		_Graph_remove_edge__doc__
	},
	{
		"get_edge",
		(PyCFunction) Graph_get_edge,
		METH_VARARGS,
		_Graph_get_edge__doc__
	},
	//
	// Neighbour data
	//
	{
		"neighbours",
		(PyCFunction) Graph_neighbours,
		METH_VARARGS | METH_KEYWORDS,
		_Graph_neighbours__doc__
	},
	{
		"degree",
		(PyCFunction) Graph_degree,
		METH_VARARGS | METH_KEYWORDS,
		_Graph_degree__doc__
	},
	//
	// Flags
	//
	{
		"get_flag",
		(PyCFunction) Graph_get_flag,
		METH_O,
		_Graph_get_flag__doc__
	},
	{
		"set_flag",
		(PyCFunction) Graph_set_flag,
		METH_VARARGS,
		_Graph_set_flag__doc__
	},
	{
		"set_all_flags",
		(PyCFunction) Graph_set_all_flags,
		METH_O,
		_Graph_set_all_flags__doc__
	},
	//
	// Sentinel
	//
	{
		NULL,
		NULL,
		0,
		NULL
	}
};

static PyTypeObject GraphType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pygraph.Graph",                                  
	.tp_basicsize = sizeof(GraphObject),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor) Graph_dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc = _Graph_init__doc__,
	.tp_methods = Graph_methods,
	.tp_init = (initproc) Graph_init,
	.tp_new = (newfunc) Graph_new,
};

PyModuleDef pygraph_module = {
	PyModuleDef_HEAD_INIT,
	"pygraph",
};

PyMODINIT_FUNC
PyInit_pygraph(void)
{
	PyObject *m;
	Py_Initialize();
	if (PyType_Ready(&GraphType) < 0)
		return NULL;
	m = PyModule_Create(&pygraph_module);
	if (m == NULL)
		return NULL;
	Py_INCREF(&GraphType);
	if (PyModule_AddObject(m, "Graph", (PyObject *) &GraphType) < 0) {
		Py_DECREF(&GraphType);
		Py_DECREF(m);
		return NULL;
	}
	return m;
}
