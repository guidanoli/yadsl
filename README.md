# Analysis of Algorithms (INF1721)
[![Build Status](https://travis-ci.com/guidanoli/aa.svg?branch=master)](https://travis-ci.com/guidanoli/aa)

This is a repository for toy projects that sparked from the lectures given by Eduardo Sany Laber, professor at PUC-Rio. I hope it comes to any help to you. Feel free to contribute to the repository too.

## C Modules

Each subfolder generally contains a source file (`module.c`), a header file (`module.h`), a test module (`module.test.c`), a test script (`module.script`), and a documentation file (`README.md`), as well as a `CMakeLists.txt` file.

### Compilation

All the compilation process is described in form of `CMakeLists.txt`. Having installed [cmake](https://cmake.org/), you may run the following commands:

```bash
mkdir build
cd build
cmake ..
cmake --build . --config=<CONFIG>
```

### Tests

All test modules make use of the `tester` and `memdb` frameworks. To read more about it, go to `tester/README.md` and `memdb/README.md`. In order to run the tests, assuming you have [cmake](https://cmake.org/) installed, simply run:

```bash
cd build
ctest -C <CONFIG>
```

Where `CONFIG` stands for the project configuration (like `Debug` or `Release`).

## Documentation details

Each function has the following fields documented:

* **name**: function name
* **description**: function description
* **parameters**: parameter description and tags
* **possible error values**: and eventual annotations

Along with a parameter there might be one or more tags associated:

* **ret**: parameter returned by reference and **not owned** by the caller
* **owned ret**: parameter returned by reference and **owned** by the caller
* **opt**: parameter (typically a pointer) can assume **`NULL`**
* **owned**: if the function succeeds, the parameter is **owned** by data structure 

**DISCLAIMER:** A parameter cannot be NULL unless otherwise said so.

## Python modules

Some modules have Python bindings, which comprehend each a source file (`module.py.c`) and a test file (`module_test.py`), as well as a `CMakeLists.txt` file. They are always contained inside the `python` folder of the corresponding C module.

### Compilation

Run the `setup.py` script, which will also properly install the python extension modules:

```bash
python setup.py install
```

### Tests

All python modules can be easily tested with `pytest`, which searches for `*_test.py` files in the whole directory. Once the modules are compiled and installed by the `setup.py`, you may run the following commands:

```bash
python -m pip install -r requirements.txt
python -m pytest
```

## Creating a new project

If you wish to create a new project following the pattern of the already existing ones, you may use the `new.py` script located on the root directory. You also choose whether to create testing code and python binding code.

```bash
python new.py [<project> [--python=[YES/NO]] [--test=[YES/NO]]]
```