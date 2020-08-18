#! bin/bash

# A script for testing lua modules

set -eo pipefail

LUA_CPATH=$PWD/lib64/?.so
find src -name '*_test.lua' -print0 | xargs -n 1 -0 $LUA_CMD
if [ $? -eq 0 ]
then
	echo -e "Lua tests passed"
else
	echo -e "Lua tests failed"
fi
