#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "pydefines.h"

#include "avl.h"
#include "memdb.h"

//
// Objects
//

typedef struct
{
	PyObject_HEAD
	AVL *ob_tree;
	PyObject *ob_func;
	unsigned char lock : 1;
} AVLObject;

//
// Exceptions
//

static PyObject *PyExc_Lock = NULL;

_DEFINE_EXCEPTION_METADATA()

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

_DEFINE_EXCEPTION_FUNCTIONS(AVL, exceptions)

//
// Callbacks
//

static void
decRefCallback(void *object)
{
	Py_XDECREF((PyObject *) object);
}

static int
cmpCallback(void *obj1, void *obj2, void *arg)
{
	int result = 1;
	AVLObject *ho;
	PyObject *callable;
	ho = (AVLObject *) arg;
	if (callable = ho->ob_func) {
		PyObject *args = NULL, *resultObj = NULL, *zero = NULL;
		if ((args = PyTuple_Pack(2, obj1, obj2)) == NULL)
			goto exit1;
		ho->lock = 1;
		resultObj = PyObject_CallObject(callable, args);
		Py_DECREF(args);
		if (resultObj == NULL) {
			if (!PyErr_Occurred())
				PyErr_SetString(PyExc_RuntimeError,
					"An unspecified error occurred during callback.");
			goto exit1;
		}
		if (!PyNumber_Check(resultObj)) {
			PyErr_Format(PyExc_TypeError,
				"Return should be a number, not %.200s",
				Py_TYPE(resultObj)->tp_name);
			goto exit1;
		}
		zero = PyLong_FromLong(0L);
		if (zero == NULL)
			goto exit1;
		result = PyObject_RichCompareBool(resultObj, zero, Py_LT);
		if (result == -1) {
			PyErr_SetString(PyExc_RuntimeError,
				"An unspecified error while comparing objects.");
			goto exit1;
		}
		if (result) {
			// result = -1	-> resultObj < 0
			result = -1;
		} else {
			result = PyObject_RichCompareBool(resultObj, zero, Py_GT);
			if (result == -1) {
				PyErr_SetString(PyExc_RuntimeError,
					"An unspecified error while comparing objects.");
				goto exit1;
			}
			// result = 0	-> obj1 == obj2
			// result = 1	-> obj1 >	obj2
		}
exit1:
		ho->lock = 0;
		Py_XDECREF(zero);
		Py_XDECREF(resultObj);
	} else {
		Py_XINCREF(obj1);
		ho->lock = 1;
		result = PyObject_RichCompareBool(obj1, obj2, Py_LT);
		if (result == -1) {
			PyErr_SetString(PyExc_RuntimeError,
				"An unspecified error while comparing objects.");
			goto exit2;
		}
		if (result) {
			// result = -1	-> obj1 <	obj2
			result = -1;
		} else {
			result = PyObject_RichCompareBool(obj1, obj2, Py_GT);
			if (result == -1) {
				PyErr_SetString(PyExc_RuntimeError,
					"An unspecified error while comparing objects.");
				goto exit2;
			}
			// result = 0	-> obj1 == obj2
			// result = 1	-> obj1 >	obj2
		}
exit2:
		ho->lock = 0;
		Py_XDECREF(obj1);
	}
	return result;
}

struct _visit_cb_arg
{
	AVLObject *ao; /* avl object */
	PyObject *func; /* callable object */
};

void *visitCallback(void *object, void *arg)
{
	struct _visit_cb_arg *info = (struct _visit_cb_arg *) arg;
	PyObject *func_arg = PyTuple_Pack(1, object), *result;
	if (func_arg == NULL) {
		PyErr_SetString(PyExc_MemoryError,
			"Could not create internal tuple");
		return arg; // Flag for 'internal error'
	}
	info->ao->lock = 1;
	result = PyObject_CallObject(info->func, func_arg);
	info->ao->lock = 0;
	Py_DECREF(func_arg);
	if (result == NULL) {
		if (!PyErr_Occurred())
			PyErr_SetString(PyExc_RuntimeError,
				"An unspecified error occurred during callback.");
		return arg; // Flag for 'internal error'
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
	AVLObject *self;
	self = (AVLObject *) type->tp_alloc(type, 0);
	if (self != NULL) {
		self->ob_tree = NULL;
		self->ob_func = NULL;
		self->lock = 0;
	}
	return (PyObject *) self;
}

PyDoc_STRVAR(_AVL_init__doc__,
"AVL(/, f=None)\n"
"--\n"
"\n"
"Python AVL tree data structure.\n"
"f is the following comparison function:\n"
"\tf(o1 : Object, o2: Object) -> int\n"
"It takes two objects o1 and o2 and returns:\n"
"\t< 0, if o1 < o2\n"
"\t= 0, if o1 = o2\n"
"\t> 0, if o1 > o2\n");

static int
AVL_init(AVLObject *self, PyObject *args, PyObject *kw)
{
	PyObject *callbackObj = NULL;
	static char *keywords[] = { "f", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kw,
		"|O:pyavl.AVL.__init__", keywords, &callbackObj))
		return -1;
	if (callbackObj != NULL) {
		if (!PyCallable_Check(callbackObj)) {
			PyErr_SetString(PyExc_TypeError,
				"f should be a callable object");
			return -1;
		}
	}
	switch (avlCreate(&self->ob_tree, cmpCallback, decRefCallback, self)) {
	case AVL_OK:
		break;
	case AVL_MEMORY:
		PyErr_SetString(PyExc_MemoryError, "Could not create avl tree");
		return -1;
	default:
		Py_UNREACHABLE();
	}
	self->ob_func = callbackObj;
	Py_XINCREF(callbackObj);
	_memdb_dump();
	return 0;
}

static void
AVL_dealloc(AVLObject *self)
{
	if (self->ob_tree)
		avlDestroy(self->ob_tree);
	_memdb_dump();
	Py_XDECREF(self->ob_func);
	Py_TYPE(self)->tp_free((PyObject *) self);
}

PyDoc_STRVAR(_AVL_add__doc__,
"add(obj : Object) -> bool\n"
"--\n"
"\n"
"Add object to tree.\n"
"Returns whether it was added or not.");

static PyObject *
AVL_add(AVLObject *self, PyObject *obj)
{
	int exists;
	if (self->lock) {
		_AVL_throw_error(PyExc_Lock);
		return NULL;
	}
	switch (avlInsert(self->ob_tree, obj, &exists)) {
	case AVL_OK:
		_memdb_dump();
		if (!exists)
			Py_INCREF(obj);
		if (PyErr_Occurred())
			return NULL;
		return PyBool_FromLong(!exists);
	case AVL_MEMORY:
		PyErr_SetString(PyExc_MemoryError,
			"Could not allocate memory for newly added item in tree");
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
AVL_remove(AVLObject *self, PyObject *obj)
{
	int exists;
	if (self->lock) {
		_AVL_throw_error(PyExc_Lock);
		return NULL;
	}
	switch (avlDelete(self->ob_tree, obj, &exists)) {
	case AVL_OK:
		_memdb_dump();
		if (PyErr_Occurred())
			return NULL;
		Py_RETURN_NONE;
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
AVL_contains(AVLObject *self, PyObject *obj)
{
	int exists;
	if (self->lock) {
		_AVL_throw_error(PyExc_Lock);
		return NULL;
	}
	switch (avlSearch(self->ob_tree, obj, &exists)) {
	case AVL_OK:
		if (PyErr_Occurred())
			return NULL;
		return PyBool_FromLong(exists);
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
AVL_iterate(AVLObject *self, PyObject *obj)
{
	struct _visit_cb_arg arg;
	void *ret = NULL;
	if (self->lock) {
		_AVL_throw_error(PyExc_Lock);
		return NULL;
	}
	if (!PyCallable_Check(obj)) {
		PyErr_SetString(PyExc_TypeError,
			"argument should be a callable object");
		return NULL;
	}
	arg.ao = self;
	arg.func = obj;
	switch (avlTraverse(self->ob_tree, visitCallback, &arg, &ret)) {
	case AVL_OK:
		_memdb_dump();
		if (PyErr_Occurred())
			goto exit;
		if (ret == NULL)
			Py_RETURN_NONE;
		return ret;
	default:
		Py_UNREACHABLE();
	}
exit:
	if (ret != &arg) // flag for 'internal error'
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
	.tp_basicsize = sizeof(AVLObject),
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
	_INIT_EXCEPTION_OBJECTS(m, _exc)
	Py_INCREF(&AVLType);
	if (PyModule_AddObject(m, "AVL", (PyObject *) &AVLType) < 0) {
		Py_DECREF(&AVLType);
		Py_DECREF(m);
		return NULL;
	}
	return m;
}
