Dependencies
============

This is an *extremely picky* list of dependencies, which means that it is
very likely that older versions of such dependencies might still work.
If you found that an older version still works, don't hesitate in sending a
`pull request <https://github.com/guidanoli/yadsl/pulls>`_.
This way, the list always is kept up-to-date.

Supported Compilers
-------------------

Since portability has always been an important aspect of **yadsl**, you should
be able to compile it in pretty much any compiler that mostly supports *C99*.
In that matter, following are the officially supported compilers.

* `GCC <https://gcc.gnu.org/>`_ >= 5.4.0
* `Clang/LLMV <https://clang.llvm.org/>`_ >= 7.0.0
* `Visual C++ <https://visualstudio.microsoft.com/vs/>`_ >= 16.7.2

Build tools
-----------

Tools necessary to generate a build system, but not compile the source code.

* `CMake <https://cmake.org/>`_ >= 3.0.0

Documentation tools
-------------------

Tools necessary to generate the library documentation, if you wish to.

* `CPython <https://www.python.org/>`_ >= 3.x
* `Breathe <https://breathe.readthedocs.io/>`_ >= 4.20.0
* `Sphinx <https://www.sphinx-doc.org/>`_ >= 3.2.1
* `Doxygen <https://www.doxygen.nl/index.html>`_ >= 1.8.19

Extension Languages
-------------------

Languages that allow dynamic linking of extension modules binding **yadsl**.

* `CPython <https://www.python.org/>`_ >= 3.5.9
* `Lua <https://www.lua.org/>`_ >= 5.3.0