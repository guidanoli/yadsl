#!/bin/bash

# A script for running test suites for travis-ci

# CMake
pushd $AA_CMAKE_DIR
ctest -V -C Release
popd

# Python
if [ "$AA_PYTHON_SUPPORT" == "ON" ]; then
	export PYTHONPATH=$PWD/lib64
	python -m pip install --user -r requirements.txt
	python -m pytest -s src
fi

# Lua
if [ "$AA_LUA_SUPPORT" == "ON" ]; then
	source luatest.sh
fi
