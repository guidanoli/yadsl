#!/bin/bash

# A script for showing software versions for travis-ci

# CMake
cmake --version

# Python
python -m pip --version

# PIP
if [ "$AA_PYTHON_SUPPORT" == "ON" ]; then
	python --version
fi

# Lua
if [ "$AA_LUA_SUPPORT" == "ON" ]; then
	$LUA -v
fi
