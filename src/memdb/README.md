# memdb

The `memdb` or Memory Debugger is a handy tool for spotting memory leaks with little to no effort. In a nutshell, it keeps track of memory blocks allocated by `malloc`, `strdup`, `calloc` and `realloc` and deallocated by `free` in a list, also holding metadata like the size of the memory block, file and line of allocation.

It works by overriding these functions with macros and extracting metadata from predefined macros like `__FILE__` and `__LINE__`. It is very simple yet handy.

You can also consult whether a memory block is in the list, get the list size, and clear it. Generally, you'd want to check whether it is empty or not, to make sure there was no memory leakage, and whether there was any error spotted.

Keep in mind that the macros are only defined when the `_DEBUG` flag is set. Some IDEs, like Visual Studio, already define this flag for Debug builds.
