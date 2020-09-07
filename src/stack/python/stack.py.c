#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <yadsl/pydefines.h>

#include <stack/stack.h>

//
// Objects
//

typedef struct
{
	PyObject_HEAD
	yadsl_StackHandle *ob_stack;
} stack_object;

//
// Exceptions
//

static PyObject *PyExc_Empty = NULL;

YADSL_PYDEFINES_EXCEPTION_METADATA()

static struct _exception_metadata exceptions[] = {
	//
	// Exception definitions
	//
	{
		&PyExc_Empty,
		"pystack.Empty",
		"Empty",
		"Stack is empty."
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

YADSL_PYDEFINES_EXCEPTION_FUNCTIONS(stack, exceptions)

//
// Callbacks
//

static void
decRefCallback(void *item)
{
	Py_XDECREF((PyObject *) item);
}

//
// Method definitions
//

static PyObject *
_stack_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
	stack_object *self;
	self = (stack_object *) type->tp_alloc(type, 0);
	if (self != NULL)
		self->ob_stack = NULL;
	return (PyObject *) self;
}

PyDoc_STRVAR(_stack_init__doc__,
"Stack(/)\n"
"--\n"
"\n"
"Python stack data structure.");

static int
_stack_init(stack_object *self, PyObject *Py_UNUSED(ignored))
{
	if (!(self->ob_stack = yadsl_stack_create())) {
		PyErr_SetString(PyExc_MemoryError, "Could not create stack");
		return -1;
	}
	return 0;
}

static void
_stack_dealloc(stack_object *self)
{
	if (self->ob_stack)
		yadsl_stack_destroy(self->ob_stack, decRefCallback);
	Py_TYPE(self)->tp_free((PyObject *) self);
}

PyDoc_STRVAR(_stackAdd__doc__,
"add(obj : Object) -> None\n"
"--\n"
"\n"
"Add object in stack.");

static PyObject *
_stackAdd(stack_object *self, PyObject *obj)
{
	switch (yadsl_stack_item_add(self->ob_stack, obj)) {
	case YADSL_STACK_RET_OK:
		Py_INCREF(obj);
		Py_RETURN_NONE;
	case YADSL_STACK_RET_MEMORY:
		PyErr_SetString(PyExc_MemoryError, "Could not add object to stack");
		break;
	default:
		Py_UNREACHABLE();
	}
	return NULL;
}

PyDoc_STRVAR(_stackRemove__doc__,
"remove() -> Object\n"
"--\n"
"\n"
"Remove object from stack.");

static PyObject *
_stackRemove(stack_object *self, PyObject *Py_UNUSED(ignored))
{
	PyObject *obj = NULL;
	switch (yadsl_stack_item_remove(self->ob_stack, &obj)) {
	case YADSL_STACK_RET_OK:
		// Borrow reference
		return obj;
	case YADSL_STACK_RET_EMPTY:
		_stack_throw_error(PyExc_Empty);
		break;
	default:
		Py_UNREACHABLE();
	}
	return NULL;
}

PyDoc_STRVAR(_stackEmpty__doc__,
"is_empty() -> bool\n"
"--\n"
"\n"
"Check if stack is empty.");

static PyObject *
_stackEmpty(stack_object *self, PyObject *Py_UNUSED(ignored))
{
	bool is_empty = 0;
	switch (yadsl_stack_empty_check(self->ob_stack, &is_empty)) {
	case YADSL_STACK_RET_OK:
		return PyBool_FromLong(is_empty);
	default:
		Py_UNREACHABLE();
	}
	return NULL;
}

//
// Method table
//

PyMethodDef stack_methods[] = {
	//
	// Objects
	//
	{
		"add",
		(PyCFunction) _stackAdd,
		METH_O,
		_stackAdd__doc__
	},
	{
		"remove",
		(PyCFunction) _stackRemove,
		METH_NOARGS,
		_stackRemove__doc__
	},
	{
		"is_empty",
		(PyCFunction) _stackEmpty,
		METH_NOARGS,
		_stackEmpty__doc__
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

static PyTypeObject stackType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pystack.stack",
	.tp_basicsize = sizeof(stack_object),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor) _stack_dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc = _stack_init__doc__,
	.tp_methods = stack_methods,
	.tp_init = (initproc) _stack_init,
	.tp_new = (newfunc) _stack_new,
};

//
// Module
//

PyModuleDef pystack_module = {
	PyModuleDef_HEAD_INIT,
	"pystack",
};

PyMODINIT_FUNC
PyInit_pystack(void)
{
	PyObject *m;
	struct _exception_metadata *_exc;
	Py_Initialize();
	if (PyType_Ready(&stackType) < 0)
		return NULL;
	m = PyModule_Create(&pystack_module);
	if (m == NULL)
		return NULL;
	YADSL_PYDEFINES_INIT_EXCEPTION_OBJECTS(m, _exc)
		Py_INCREF(&stackType);
	if (PyModule_AddObject(m, "stack", (PyObject *) &stackType) < 0) {
		Py_DECREF(&stackType);
		Py_DECREF(m);
		return NULL;
	}
	return m;
}