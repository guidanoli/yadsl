#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <assert.h>

#include <yadsl/py.h>
#include <avl/avl.h>

#ifdef YADSL_DEBUG
#include <memdb/memdb.h>
#else
#include <stdlib.h>
#endif

//
// Objects
//

typedef struct
{
	PyObject_HEAD
	yadsl_AVLTreeHandle *ob_tree;
	unsigned char lock : 1;
} yadsl_AVLTreePythonObject;

//
// Exceptions
//

static PyObject *PyExc_Lock = NULL;

YADSL_PY_EXCEPTION_METADATA()

static struct _exception_metadata exceptions[] = {
	//
	// Exception definitions
	//
	{
		&PyExc_Lock,
		"pyavl.Lock",
		"Lock",
		"Tree is locked. Illegal operation detected."
	},
	//
	// Sentinel
	//
	{
		NULL,
		NULL,
		NULL,
	},
};

YADSL_PY_EXCEPTION_FUNCTIONS(yadsl_AVLTreePythonObject, exceptions)

//
// Callbacks
//

static void
decRefCallback(void *object, void *arg)
{
	Py_XDECREF((PyObject *) object);
}

static yadsl_AVLTreeComparison
cmpCallback(void *obj1, void *obj2, void *cmp_objs_arg)
{
	int result;
	yadsl_AVLTreeComparison cmp;
	yadsl_AVLTreePythonObject *ho;
	ho = (yadsl_AVLTreePythonObject *) cmp_objs_arg;
	Py_XINCREF(obj1);
	Py_XINCREF(obj2);
	ho->lock = 1;
	result = PyObject_RichCompareBool(obj1, obj2, Py_EQ);
	if (result == -1) {
		cmp = YADSL_AVLTREE_COMP_ERR;
	} else if (result) {
		cmp = YADSL_AVLTREE_COMP_EQ;
	} else {
		result = PyObject_RichCompareBool(obj1, obj2, Py_LT);
		if (result == -1) {
			cmp = YADSL_AVLTREE_COMP_ERR;
		} else if (result) {
			cmp = YADSL_AVLTREE_COMP_LT;
		} else {
			cmp = YADSL_AVLTREE_COMP_GT;
		}
	}
	ho->lock = 0;
	Py_XDECREF(obj2);
	Py_XDECREF(obj1);
	return cmp;
}

struct _visit_cb_arg
{
	yadsl_AVLTreePythonObject *ao; /* avl object */
	PyObject *func; /* callable object */
};

void *visitCallback(void *object, void *visitArg)
{
	struct _visit_cb_arg *info = (struct _visit_cb_arg *) visitArg;
	PyObject *func_arg = PyTuple_Pack(1, object), *result;
	if (func_arg == NULL) {
		PyErr_SetString(PyExc_MemoryError,
			"Could not create internal tuple");
		return visitArg; // Flag for 'internal error'
	}
	info->ao->lock = 1;
	result = PyObject_CallObject(info->func, func_arg);
	info->ao->lock = 0;
	Py_DECREF(func_arg);
	if (result == NULL) {
		if (!PyErr_Occurred())
			PyErr_SetString(PyExc_RuntimeError,
				"An unspecified error occurred during callback.");
		return visitArg; // Flag for 'internal error'
	}
	if (!PyObject_IsTrue(result)) {
		Py_DECREF(result);
		return NULL;
	}
	return result; // Return to function
}

//
// Method definitions
//

static PyObject *
AVL_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
	yadsl_AVLTreePythonObject *self;
	self = (yadsl_AVLTreePythonObject *) type->tp_alloc(type, 0);
	if (self != NULL) {
		self->ob_tree = NULL;
		self->lock = 0;
	}
	return (PyObject *) self;
}

PyDoc_STRVAR(_AVL_init__doc__,
"AVL()\n"
"--\n"
"\n"
"Python AVL tree data structure.\n");

static int
AVL_init(yadsl_AVLTreePythonObject *self, PyObject *args, PyObject *kw)
{
	if (!(self->ob_tree = yadsl_avltree_tree_create())) {
		PyErr_SetString(PyExc_MemoryError, "Could not create avl tree");
		return -1;
	}
#ifdef YADSL_DEBUG
	yadsl_memdb_status();
#endif
	return 0;
}

static void
AVL_dealloc(yadsl_AVLTreePythonObject *self)
{
	if (self->ob_tree) {
		yadsl_AVLTreeCallbacks callbacks = {.free_cb = decRefCallback};
		yadsl_avltree_destroy(self->ob_tree, &callbacks);
	}
#ifdef YADSL_DEBUG
	yadsl_memdb_status();
#endif
	Py_TYPE(self)->tp_free((PyObject *) self);
}

PyDoc_STRVAR(_AVL_add__doc__,
"add(obj : Object) -> bool\n"
"--\n"
"\n"
"Add object to tree.\n"
"Returns whether it was added or not.");

static PyObject *
AVL_add(yadsl_AVLTreePythonObject *self, PyObject *obj)
{
	bool exists;
	if (self->lock) {
		_yadsl_AVLTreePythonObject_throw_error(PyExc_Lock);
		return NULL;
	}
	yadsl_AVLTreeCallbacks callbacks = {.compare_cb = cmpCallback, .compare_arg = self};
	switch (yadsl_avltree_object_insert(self->ob_tree, obj, &callbacks, &exists)) {
	case YADSL_AVLTREE_RET_OK:
#ifdef YADSL_DEBUG
		yadsl_memdb_status();
#endif
		if (!exists)
			Py_INCREF(obj);
		assert(!PyErr_Occurred());
		return PyBool_FromLong(!exists);
	case YADSL_AVLTREE_RET_MEMORY:
		PyErr_SetString(PyExc_MemoryError,
			"Could not allocate memory for newly added item in tree");
		break;
	case YADSL_AVLTREE_RET_ERR:
		assert(PyErr_Occurred());
		break;
	default:
		Py_UNREACHABLE();
	}
	return NULL;
}

PyDoc_STRVAR(_AVL_remove__doc__,
"remove(obj : Object) -> None\n"
"--\n"
"\n"
"Remove object from tree.");

static PyObject *
AVL_remove(yadsl_AVLTreePythonObject *self, PyObject *obj)
{
	bool exists;
	if (self->lock) {
		_yadsl_AVLTreePythonObject_throw_error(PyExc_Lock);
		return NULL;
	}
	yadsl_AVLTreeCallbacks callbacks = {.compare_cb = cmpCallback, .compare_arg = self, .free_cb = decRefCallback};
	switch (yadsl_avltree_object_remove(self->ob_tree, obj, &callbacks, &exists)) {
	case YADSL_AVLTREE_RET_OK:
#ifdef YADSL_DEBUG
		yadsl_memdb_status();
#endif
		assert(!PyErr_Occurred());
		Py_RETURN_NONE;
	case YADSL_AVLTREE_RET_ERR:
		assert(PyErr_Occurred());
		break;
	default:
		Py_UNREACHABLE();
	}
	return NULL;
}

PyDoc_STRVAR(_AVL_contains__doc__,
"contains(obj : Object) -> bool\n"
"--\n"
"\n"
"Check whether tree contains object.");

static PyObject *
AVL_contains(yadsl_AVLTreePythonObject *self, PyObject *obj)
{
	bool exists;
	if (self->lock) {
		_yadsl_AVLTreePythonObject_throw_error(PyExc_Lock);
		return NULL;
	}
	yadsl_AVLTreeCallbacks callbacks = {.compare_cb = cmpCallback, .compare_arg = self};
	switch (yadsl_avltree_object_search(self->ob_tree, obj, &callbacks, &exists)) {
	case YADSL_AVLTREE_RET_OK:
		assert(!PyErr_Occurred());
		return PyBool_FromLong(exists);
	case YADSL_AVLTREE_RET_ERR:
		assert(PyErr_Occurred());
		break;
	default:
		Py_UNREACHABLE();
	}
	return NULL;
}

PyDoc_STRVAR(_AVL_iterate__doc__,
"iterate(f : Object) -> Object\n"
"--\n"
"\n"
"Iterate through tree in order with a function.\n"
"If a value other than None/False is returned, the\n"
"iteration will be interrupted and that will be the\n"
"returned value of iterate. Else, None is returned.");

static PyObject *
AVL_iterate(yadsl_AVLTreePythonObject *self, PyObject *obj)
{
	struct _visit_cb_arg visitArg;
	void *ret = NULL;
	if (self->lock) {
		_yadsl_AVLTreePythonObject_throw_error(PyExc_Lock);
		return NULL;
	}
	if (!PyCallable_Check(obj)) {
		PyErr_SetString(PyExc_TypeError,
			"argument should be a callable object");
		return NULL;
	}
	visitArg.ao = self;
	visitArg.func = obj;
	yadsl_AVLTreeCallbacks callbacks = {.visit_cb = visitCallback, .visit_arg = &visitArg};
	switch (yadsl_avltree_tree_traverse(self->ob_tree, YADSL_AVLTREE_VISITING_IN_ORDER, &callbacks, &ret)) {
	case YADSL_AVLTREE_RET_OK:
#ifdef YADSL_DEBUG
		yadsl_memdb_status();
#endif
		if (PyErr_Occurred())
			goto exit;
		if (ret == NULL)
			Py_RETURN_NONE;
		return ret;
	default:
		Py_UNREACHABLE();
	}
exit:
	if (ret != &visitArg) // flag for 'internal error'
		Py_XDECREF(ret);
	return NULL;
}

//
// Method table
//

PyMethodDef AVL_methods[] = {
	//
	// Methods
	//
	{
		"add",
		(PyCFunction) AVL_add,
		METH_O,
		_AVL_add__doc__
	},
	{
		"remove",
		(PyCFunction) AVL_remove,
		METH_O,
		_AVL_remove__doc__
	},
	{
		"contains",
		(PyCFunction) AVL_contains,
		METH_O,
		_AVL_contains__doc__
	},
	{
		"iterate",
		(PyCFunction) AVL_iterate,
		METH_O,
		_AVL_iterate__doc__
	},
	//
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

//
// Types
//

static PyTypeObject AVLType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pyavl.AVL",
	.tp_basicsize = sizeof(yadsl_AVLTreePythonObject),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor) AVL_dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc = _AVL_init__doc__,
	.tp_methods = AVL_methods,
	.tp_init = (initproc) AVL_init,
	.tp_new = (newfunc) AVL_new,
};

//
// Module
//

PyModuleDef pyavl_module = {
	PyModuleDef_HEAD_INIT,
	"pyavl",
};

PyMODINIT_FUNC
PyInit_pyavl(void)
{
	PyObject *m;
	struct _exception_metadata *_exc;
	Py_Initialize();
	if (PyType_Ready(&AVLType) < 0)
		return NULL;
	m = PyModule_Create(&pyavl_module);
	if (m == NULL)
		return NULL;
	YADSL_PY_INIT_EXCEPTION_OBJECTS(m, _exc)
	Py_INCREF(&AVLType);
	if (PyModule_AddObject(m, "AVL", (PyObject *) &AVLType) < 0) {
		Py_DECREF(&AVLType);
		Py_DECREF(m);
		return NULL;
	}
	return m;
}
