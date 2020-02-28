include(addPythonModule)
add_python_module(pyavl pyavl avl.py.c)
target_link_libraries(pyavl avl common)
