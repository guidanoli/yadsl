#include <Python.h>
#include "queue.h"

 static PyObject* get10(PyObject* self, PyObject* args) {
     return PyLong_FromLong(10L);
 }

PyMethodDef method_table[] = {
     {"get10", (PyCFunction) get10, METH_VARARGS, "Get 10"},
    {NULL, NULL, 0, NULL} // Sentinel value ending the table
};

PyModuleDef pyqueue_module = {
    PyModuleDef_HEAD_INIT,
    "pyqueue", // Module name
    "Python binding of the queue module",
    -1,   // Optional size of the module state memory
    method_table
};

// The module init function
PyMODINIT_FUNC PyInit_pyqueue(void) {
    Py_Initialize();
    return PyModule_Create(&pyqueue_module);
}
