# yadsl - Yet Another Data Structure Library
[![Build Status](https://travis-ci.com/guidanoli/yadsl.svg?branch=master)](https://travis-ci.com/guidanoli/yadsl)

This is a repository for toy projects that sparked from the Analysis of Algorithms (INF1721) lectures given by Eduardo Sany Laber, professor at PUC-Rio. I hope it comes to any help to you. Feel free to contribute to the repository too.

## C Modules

Each subfolder generally contains a source file (`module.c`), a header file (`module.h`), a test module (`module.test.c`), a test script (`module.script`), and a documentation file (`README.md`), as well as a `CMakeLists.txt` file.

### Compilation

All the compilation process is described in form of `CMakeLists.txt`. Having installed [cmake](https://cmake.org/), you may run the following commands:

```sh
mkdir build
cd build
cmake ..
cmake --build . --config=<CONFIG>
```

or, alternatively, you can use the `setup.py` script. You can use environment variables to control your options too.

```sh
export YADSL_PYTHON_SUPPORT=ON
python setup.py install
```

### Tests

All test modules make use of the `tester` and `memdb` frameworks. To read more about it, go to `tester/README.md` and `memdb/README.md`. In order to run the tests, assuming you have [cmake](https://cmake.org/) installed, simply run:

```sh
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

```sh
python setup.py install
```

### Tests

All python modules can be easily tested with `pytest`, which searches for `*_test.py` files in the whole directory. Once the modules are compiled and installed by the `setup.py`, you may run the following commands:

```sh
export PYTHONPATH=$PWD/lib64
python -m pip install -r requirements.txt
python -m pytest
```

## Lua modules

Some modules have Lua bindings, which comprehend each a source file (`module.lua.c`) and a test file (`module_test.lua`), as well as a `CMakeLists.txt` file. They are always contained inside the `lua` folder of the corresponding C module.

### Compilation

Run the `setup.py` script, with the environment variable `YADSL_LUA_SUPPORT` set to `ON`.
If you don't want to generate the python bindings, just set `YADSL_PYTHON_SUPPORT` to `OFF`.

#### POSIX

```sh
export YADSL_LUA_SUPPORT=ON
export YADSL_PYTHON_SUPPORT=OFF
python setup.py install
```

#### Windows

```dos
set YADSL_LUA_SUPPORT=ON
set YADSL_PYTHON_SUPPORT=OFF
python setup.py install
```

### Tests

All lua modules can be easily tested with `luatest`, which searches for `*_test.lua` files in the whole directory. Once the modules are compiled, you may run the following command:

Make sure to set the LUA environment variable to the Lua interpreter name available on your machine.

#### POSIX

```sh
export LUA=lua54
source luatest.sh
```

#### Windows

```dos
set LUA=lua54
luatest
```

## Creating a new project

If you wish to create a new project following the pattern of the already existing ones, you may use the `add_new_project.py` script located on the `config` directory.
