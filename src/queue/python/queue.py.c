#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <queue/queue.h>

typedef struct {
    PyObject_HEAD
    Queue* ob_queue;
} QueueObject;

static void
Queue_decRefCallback(void *item)
{
	Py_DECREF((PyObject *) item);
}

static PyObject *
Queue_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
	QueueObject *self;
	self = (QueueObject *) type->tp_alloc(type, 0);
	if (self != NULL) {
		if (queueCreate(&self->ob_queue, Queue_decRefCallback)) {
			Py_DECREF(self);
			return NULL;
		}
	}
	return (PyObject *) self;
}

static void
Queue_dealloc(QueueObject *self)
{
	queueDestroy(self->ob_queue);
	Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
Queue_queue(QueueObject *self, PyObject *args, PyObject *kw)
{
	PyObject *obj;
	if (!PyArg_ParseTuple(args, "O:Queue.queue", &obj))
		return NULL;
	if (obj) {
		if (queueQueue(self->ob_queue, obj))
			return PyErr_NoMemory();
		Py_INCREF(obj);
	}
	Py_RETURN_NONE;
}

static PyObject *
Queue_dequeue(QueueObject *self, PyObject *args, PyObject *kw)
{
	PyObject *obj;
	int is_empty;
	if (queueIsEmpty(self->ob_queue, &is_empty))
		return NULL;
	if (is_empty) {
		PyErr_SetString(PyExc_RuntimeError, "Empty queue");
		return NULL; // Throw exception
	}
	if (queueDequeue(self->ob_queue, &obj))
		return NULL;
	Py_DECREF(obj);
	return obj;
}

static PyObject *
Queue_is_empty(QueueObject *self, PyObject *args, PyObject *kw)
{
	int is_empty;
	if (queueIsEmpty(self->ob_queue, &is_empty))
		return NULL;
	return PyBool_FromLong(is_empty);
}

static PyObject *
Queue_next(QueueObject *self)
{
	PyObject *obj;
	int is_empty;
	if (queueIsEmpty(self->ob_queue, &is_empty))
		return NULL;
	if (is_empty)
		return NULL; // Stop iteration
	if (queueDequeue(self->ob_queue, &obj))
		return NULL;
	return obj; // Pass ownership to iterator
}

static PyMethodDef Queue_methods[] = {
	{"queue", (PyCFunction) Queue_queue, METH_VARARGS, "Queue object"},
	{"dequeue", (PyCFunction) Queue_dequeue, METH_NOARGS, "Dequeue and return object"},
	{"is_empty", (PyCFunction) Queue_is_empty, METH_NOARGS, "Check if queue is empty"},
	{NULL}
};

static PyTypeObject QueueType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pyqueue.Queue",
	.tp_doc = "Queue object (FIFO list)",
	.tp_basicsize = sizeof(QueueObject),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_new = Queue_new,
	.tp_dealloc = (destructor) Queue_dealloc,
	.tp_methods = Queue_methods,
	.tp_iter = PyObject_SelfIter,
	.tp_iternext = (iternextfunc) Queue_next,
};

PyModuleDef pyqueue_module = {
	PyModuleDef_HEAD_INIT,
	"pyqueue",
};

PyMODINIT_FUNC
PyInit_pyqueue(void)
{
	PyObject *m;
	Py_Initialize();
	if (PyType_Ready(&QueueType) < 0)
		return NULL;
	m = PyModule_Create(&pyqueue_module);
	if (m == NULL)
		return NULL;
	Py_INCREF(&QueueType);
	if (PyModule_AddObject(m, "Queue", (PyObject *) &QueueType) < 0) {
		Py_DECREF(&QueueType);
		Py_DECREF(m);
		return NULL;
	}
	return m;
}
