#include <Python.h>
#include "diff.h"

static PyObject* pydiff(PyObject* self, PyObject* args) {
    char* s1, * s2;
    unsigned long result;
    if (!PyArg_ParseTuple(args, "ss", &s1, &s2))
        return NULL;
    result = diff(s1, s2);
    return PyLong_FromUnsignedLong(result);
}

PyMethodDef method_table[] = {
    {"diff", (PyCFunction) pydiff, METH_VARARGS, "Calculate diff between two string"},
    {NULL, NULL, 0, NULL} // Sentinel value ending the table
};

PyModuleDef pydiff_module = {
    PyModuleDef_HEAD_INIT,
    "pydiff", // Module name
    "Python binding of the diff module",
    -1,   // Optional size of the module state memory
    method_table
};

// The module init function
PyMODINIT_FUNC PyInit_pydiff(void) {
    Py_Initialize();
    return PyModule_Create(&pydiff_module);
}
