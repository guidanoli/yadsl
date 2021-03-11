#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <yadsl/py.h>

#include <%name%/%name%.h>

PyMethodDef method_table[] = {
    {NULL, NULL, 0, NULL}
};

PyModuleDef %name%_module = {
    PyModuleDef_HEAD_INIT,
    "py%name%",
    "Python binding of the %name% module",
    -1,
    method_table
};

PyMODINIT_FUNC PyInit_py%name%(void)
{
    Py_Initialize();
    return PyModule_Create(&py%name%_module);
}
