set -e

build_and_run_c_tests() {
	local config="$1"; shift
	
	args=()
	if [[ -n "$PYENV_VERSION" ]]; then
		args+=( "-DAA_PYTHON_SUPPORT=ON" )
		args+=( "-DPYTHON_EXECUTABLE=$(pyenv which python)" )
	fi
	if [[ -n "$LUA_VERSION" ]]; then
		args+=( "-DAA_LUA_SUPPORT=ON" )
	fi
	
	mkdir -p build_$config
	pushd build_$config
	cmake .. -DCMAKE_BUILD_TYPE=$config "${args[@]}"
	cmake --build . --config $config
	ctest . -C $config
	popd
}

run_python_tests() {
	if [[ -n "$PYENV_VERSION" ]]; then
		export PYTHONPATH=$PWD/lib64
		pytest -s
	fi
}

run_lua_tests() {
	if [[ -n "$LUA_VERSION" ]]; then
		export LUA_CPATH=$PWD/lib64/?.so
		echo "*** Running Lua tests"
		total=0
		failed=0
		for lua_test in $(find -name '*_test.lua');
		do
			if lua $lua_test ; then
				echo "*** $lua_test passed"
			else
				echo "*** $lua_test failed"
				((++failed))
			fi
			((++total))
		done
		if [[ $failed -eq 0 ]]; then
			echo "*** All $total Lua test(s) passed"
		else
			echo "*** $failed Lua test(s) failed"
			return 1
		fi
	fi
}

build_and_run_c_tests Release
run_python_tests
run_lua_tests
build_and_run_c_tests Debug

set +e
