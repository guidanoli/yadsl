include(addPythonModule)
add_python_module(pyheap pyheap heap.py.c)
target_link_libraries(pyheap heap common)