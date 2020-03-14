include(addPythonModule)
add_python_module(pystack pystack stack.py.c)
target_link_libraries(pystack stack aa)
