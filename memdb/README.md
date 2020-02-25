# memdb

The `memdb` or Memory Debugger is a handy tool for spotting memory leaks with little to no effort. In a nutshell, it keeps track of memory blocks allocated by `malloc`, `calloc` and `realloc` and delocated by `free` in a list, holding also metadata like the size of the memory block, file and line in which the block was allocated.

It works by overriding these functions with macros and extracting metadata from predefined macros like `__FILE__` and `__LINE__`. It is very simple, yet very effective.

You can also consult whether a memory block is in list, get the list size, and clear it. Generally you'd want to check whether the list is empty or not, to make sure there were no memory leakage.

Additionally, if you do not want the said functions overriden, you can set a flag `__MEMDB_SUPRESS_MACROS__` before including the `memdb.h` header. And if you don't want the leaking addressed spit out to `stderr`, you can toggle it by calling `_memdb_supress_messages(1)`.