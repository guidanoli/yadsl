local memdb = require "memdb"

local driver = {}

function driver:runmodule(module)
	assert(type(module) == "table", "expected module to be a table")
	return self:_runmodule(module)
end

function driver:_runmodule(module)
	local passed = 0
	local failed = 0
	local errors = {}
	for name, func in pairs(module) do
		if type(name) == "string" and name:find("^test") and type(func) == "function" then
			io.stderr:write("#", (passed + failed + 1), "\t", name, " ... ")
			local ok, err = xpcall(function() func(module) end, debug.traceback)
			if ok then
				io.stderr:write("\27[32mPASS\27[0m\n")
				passed = passed + 1
			else
				io.stderr:write("\27[31mFAIL\27[0m\n")
				failed = failed + 1
				errors[passed + failed] = err
			end
		end
	end
	return failed, passed, errors
end

function driver:collectallgarbage()
	local count = collectgarbage('count')
	for i = 1, 100 do
		collectgarbage('collect')
		local newcount = collectgarbage('count')
		if count == newcount then
			return newcount
		end
		count = newcount
	end
	error "Exceeded limit of garbage collection cycles"
end

function driver:runscript(script)
	assert(type(script) == "string", "expected script name to be a string")
	return self:_runscript(script)
end

function driver:_runscript(script)
	local module = require(script)
	assert(type(module) == "table", "test script should return a table")
	io.stderr:write('Test script: ', script, '\n')
	io.stderr:write(string.rep('=', 80), '\n')
	local failed, passed, errors = driver:runmodule(module)
	io.stderr:write(string.rep('=', 80), '\n')
	local haserror = false
	for i = 1, (failed + passed) do
		local err = errors[i]
		if err ~= nil then
			if haserror then
				io.stderr:write(string.rep('-', 80), '\n')
			end
			io.stderr:write("Error #", i, ":\n", tostring(err), '\n')
			haserror = true
		end
	end
	if haserror then
		io.stderr:write(string.rep('=', 80), '\n')
	end
	if failed ~= 0 then
		return false, failed .. " tests failed"
	end
	driver:collectallgarbage()
	local list = memdb.get_amb_list()
	if #list ~= 0 then
		for i, v in ipairs(list) do
			io.stderr:write(v.address, ' (', v.size, ' B) @ ', v.filename, ':', v.line , ' (', v.funcname, ')\n')
		end
		return false, #list .. " memory leaks detected"
	end
	return true
end

if type(arg) == "table" and arg[0]:find("driver%.lua$") then
	local script = assert(arg[1], "missing test script")
	assert(driver:runscript(script))
end

return driver
