#! /bin/bash

# A script for setting up a Python interpreter

set -eufo pipefail

source .travis/platform.sh

PY_VERSION=$PY_MAJOR.$PY_MINOR.$PY_PATCH

case $PLATFORM in
	linux)
		if sudo apt-get install -y $PY_CMD ; then
			echo "Installed $($PY_CMD -V) from apt-get"
		else
			PY_HOME_DIR=$TRAVIS_BUILD_DIR/install/python

			mkdir -p "$PY_HOME_DIR"

			sudo apt-get install -y build-essential zlib1g-dev libncurses5-dev libgdbm-dev \
			libnss3-dev libssl-dev libreadline-dev libffi-dev wget

			pushd "$PY_HOME_DIR"

			wget https://www.python.org/ftp/python/$PY_VERSION/Python-$PY_VERSION.tgz
			tar xzvf Python-$PY_VERSION.tgz
			cd Python-$PY_VERSION
			./configure --enable-optimizations --with-ensurepip=upgrade
			make -j $(nproc)
			sudo make altinstall

			popd

			echo "Installed $($PY_CMD -V) from source"
		fi
		export PY_CMD=python$PY_MAJOR.$PY_MINOR
		;;
	windows)
		choco install python --version $PY_VERSION
        	export PATH=/c/Python$PY_MAJOR$PY_MINOR:/c/Python$PY_MAJOR$PY_MINOR/Scripts:$PATH
		export PY_CMD=python
		echo "Installed $($PY_CMD -V) from chocolatey"
		;;
	*)
		echo "Unsupported platform \"$PLATFORM\""
		exit 1
		;;
esac
