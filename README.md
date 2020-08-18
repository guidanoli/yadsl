# Analysis of Algorithms (INF1721)
[![Build Status](https://travis-ci.com/guidanoli/aa.svg?branch=master)](https://travis-ci.com/guidanoli/aa)

This is a repository for toy projects that sparked from the lectures given by [Eduardo Sany Laber](http://www.inf.puc-rio.br/blog/professor/@eduardo-sany-laber), professor at [PUC-Rio](https://www.puc-rio.br/index.html). I hope it comes to any help to you. Feel free to contribute to the repository too.

## Creating a new project

If you wish to create a new project following the pattern of the already existing ones, you may run the `config/add_new_project.py` script.

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

## C Modules

Any module has its files located in a `src/<module-name>` folder. There you can find not only the module code, but also test code and bindings to other languages.

### Compilation

All the compilation process is described in form of CMake lists. Having installed [CMake](https://cmake.org/), you may run the following commands:

```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=<CONFIG> [other options]
cmake --build . --config=<CONFIG>
```

### Tests

All test modules make use of the `tester` and `memdb` frameworks. To read more about it, go to `tester/README.md` and `memdb/README.md`. In order to run the tests, assuming you have [CMake](https://cmake.org/) installed, simply run:

```sh
cd build
ctest -C <CONFIG>
```

## Python modules

Some C modules have Python bindings, along with a test script. They are generally contained inside the `python` folder of the corresponding C module.

### Compilation

Simply enable the `AA_PYTHON_SUPPORT` option.

```sh
cmake .. -DAA_PYTHON_SUPPORT=ON [other options]
```

If you are having a hard time trying to make CMake point to a specific Python version, worry not! You can set the `PYTHON_EXECUTABLE` option.

```sh
cmake .. -DPYTHON_EXECUTABLE=$(pyenv which python) [other options]
```

### Tests

All python modules can be easily tested with `pytest`, which searches for `*_test.py` files in the whole directory. After building the extension modules, you may run the following commands, from the root of the repository:

```sh
export PYTHONPATH=$PWD/lib64
pip install -r config/dev-requirements.txt
python -m pytest -s --ignore=config/templates
```

## Lua modules

Some C modules have Lua bindings, along with a test script. They are generally contained inside the `lua` folder of the corresponding C module.

### Compilation

Simply enable the `AA_LUA_SUPPORT` option.

```sh
cmake .. -DAA_LUA_SUPPORT=ON [other options]
```

### Tests (TODO)

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
