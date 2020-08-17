#!/bin/bash

# A script for showing software versions for travis-ci

# CMake
cmake --version

# Python
python --version

# PIP
python -m pip --version

# Lua
if [ "$AA_LUA_SUPPORT" == "ON" ]; then
	$LUA -v
fi
