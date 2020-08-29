#! /bin/bash

set -eo pipefail

source config/platform.sh

TEMP_DIR=$(mktemp -d)

pushd $TEMP_DIR

LUA_URL=http://www.lua.org/ftp/lua-$LUA_VERSION.tar.gz
echo "*** Installing Lua $LUA_VERSION source code from $LUA_URL"
curl $LUA_URL | tar xz

cd lua-$LUA_VERSION

# Build Lua without backwards compatibility for testing
perl -i -pe 's/-DLUA_COMPAT_(ALL|5_2)//' src/Makefile

CFLAGS="-fPIC"
echo "*** Compiling Lua $LUA_VERSION with options $CFLAGS"
make MYCFLAGS=$CFLAGS $PLATFORM

INSTALL_DIR=$(pwd)
echo "*** Installing Lua $LUA_VERSION in $INSTALL_DIR"
make INSTALL_TOP=$INSTALL_DIR install;

mkdir -p $LUA_DIR
mv * $LUA_DIR

popd

rm -rf $TEMP_DIR
