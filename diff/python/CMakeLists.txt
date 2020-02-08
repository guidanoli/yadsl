include(addPythonModule)
add_python_module(pydiff pydiff diff.py.c)
target_link_libraries(pydiff diff)