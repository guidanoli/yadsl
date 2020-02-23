// Python definitions header file
// Make sure to include this after including the Python headers

#ifndef Py_UNREACHABLE
#pragma message("Py_UNREACHABLE was not found, maybe you're using a Python " \
"of version previous to v3.7. It will be then defined as a macro for abort().")
#define Py_UNREACHABLE() abort()
#endif