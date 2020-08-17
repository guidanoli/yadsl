#!/bin/bash

# A script for showing software versions for travis-ci

# CMake
cmake --version

# Python
python --version || python3 --version

# PIP
pip3 --version

# Lua
if [ "$AA_LUA_SUPPORT" == "ON" ]; then
	$LUA -v
fi
