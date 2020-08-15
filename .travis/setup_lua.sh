#! /bin/bash

# A script for setting up environment for travis-ci testing

set -eufo pipefail

source .travis/platform.sh

LUA_HOME_DIR=$TRAVIS_BUILD_DIR/install/lua

mkdir $HOME/.lua

mkdir -p "$LUA_HOME_DIR"

LUA_VERSION=$LUA_MAJOR.$LUA_MINOR.$LUA_PATCH

curl http://www.lua.org/ftp/lua-$LUA_VERSION.tar.gz | tar xz
cd lua-$LUA_VERSION;

# Build Lua without backwards compatibility for testing
perl -i -pe 's/-DLUA_COMPAT_(ALL|5_2)//' src/Makefile
make MYCFLAGS="-fPIC" $PLATFORM
make INSTALL_TOP="$LUA_HOME_DIR" install;

ln -s $LUA_HOME_DIR/bin/lua $HOME/.lua/lua
ln -s $LUA_HOME_DIR/bin/luac $HOME/.lua/luac;

export LUA=$TRAVIS_BUILD_DIR/lua
