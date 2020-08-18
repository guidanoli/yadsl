#! /bin/bash

# A script for setting up a Python interpreter

set -eufo pipefail

source .travis/platform.sh

PY_VERSION=$PY_MAJOR.$PY_MINOR.$PY_PATCH

case $PLATFORM in
	linux)
		PY_CMD=python$PY_MAJOR.$PY_MINOR
		sudo apt update -y
		sudo add-apt-repository -y ppa:deadsnakes/ppay
		sudo apt install -y $PY_CMD
		echo "Installed $(${PY_CMD} -V) from apt"
		;;
	windows)
		choco install python --version $PY_VERSION
        	PATH=/c/Python$PY_MAJOR$PY_MINOR:/c/Python$PY_MAJOR$PY_MINOR/Scripts:$PATH
		PY_CMD=python
		echo "Installed $(${PY_CMD} -V) from chocolatey"
		;;
	*)
		echo "Unsupported platform \"$PLATFORM\""
		exit 1
		;;
esac
AA_PYTHON_SUPPORT=ON
