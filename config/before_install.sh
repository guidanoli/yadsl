#!/bin/bash
if [[ -n "$PYENV_VERSION" ]]; then
	PYENV_ROOT=
	source config/setup_pyenv.sh
fi
if [[ -n "$LUA_VERSION" ]]; then
	export LUA_DIR=$HOME/.lua/$LUA_VERSION
	
	if [[ -d "$LUA_DIR" ]]; then
		echo "*** Lua $LUA_VERSION was found in $LUA_DIR"
	else
		echo "*** Lua $LUA_VERSION was not found in $LUA_DIR"
		if bash config/setup_lua.sh ; then
			echo "*** Lua $LUA_VERSION installed with success in $LUA_DIR"
		else
			echo "*** Lua $LUA_VERSION could not be installed"
			return 1
		fi
	fi

	# If LUA_DIR is already in PATH, don't add to it again
	if ! echo "$PATH" | grep "$LUA_DIR/bin" &> /dev/null ; then
		export PATH=$LUA_DIR/bin:$PATH
	fi

fi
