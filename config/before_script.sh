#!/bin/bash 
cmake --version
if [[ -n "$PYENV_VERSION" ]]; then
	python --version
	pip install --upgrade pip
	pip --version
	pip install -r config/dev-requirements.txt
	PYTHON_EXEC=$(pyenv which python)
fi
if [[ -n "$LUA_VERSION" ]]; then
	hererocks --version
	lua -v
fi
