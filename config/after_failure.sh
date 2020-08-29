#!/bin/bash
dump_cmake_data() {
	local config=$1; shift
	echo "*** Dumping $config cache"
	cat $config/CMakeCache.txt
	echo "*** Dumping $config log"
	cat $config/CMakeFiles/CMakeOutput.log
}

for config in build*
do
	dump_cmake_data $config
done
