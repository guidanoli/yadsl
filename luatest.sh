export LUA_CPATH=$PWD/lib64/?.so
find . -name 'src/*_test.lua' -exec $LUA {} \;