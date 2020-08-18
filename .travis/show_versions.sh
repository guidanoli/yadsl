#!/bin/bash

# A script for showing software versions for travis-ci

# CMake
cmake --version

# Python
$PY_CMD -V

# PIP
$PY_CMD -m pip -V

# Lua
if [ "$AA_LUA_SUPPORT" == "ON" ]; then
	$LUA -v
fi
