#!/bin/bash

# A script for running test suites for travis-ci

set -eof pipefail

# CMake
pushd $(cat build_dir.cfg)
ctest -V -C Release
popd

# Python
if [ "$AA_PYTHON_SUPPORT" == "ON" ]; then
	PYTHONPATH=$PWD/lib64
        $PY_CMD -m pip install --upgrade pip
	$PY_CMD -m pip install --user -r requirements.txt
	$PY_CMD -m pytest -s --ignore=config/templates
fi

# Lua
if [ "$AA_LUA_SUPPORT" == "ON" ]; then
	source luatest.sh
fi
