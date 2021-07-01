local memdb = require "memdb"

local driver = {}

function driver:runmodule(module, errhdlr, fp)
	assert(type(module) == "table")
	errhdlr = errhdlr or debug.traceback
	fp = fp or io.stderr
	return self:_runmodule(module, errhdlr, fp)
end

function driver:_runmodule(module, errhdlr, fp)
	local passed = 0
	local failed = 0
	local errors = {}
	local function optcall(methodname, key)
		key = key or methodname
		local method = rawget(module, methodname)
		if type(method) == 'function' then
			return xpcall(function() method(module) end, errhdlr)
		else
			return true
		end
	end
	local ok, err = optcall("beforeAll")
	if not ok then
		failed = failed + 1
		errors.beforeAll = err
		return failed, passed, errors
	end
	for name, func in pairs(module) do
		if type(name) == "string" and name:find("^test") and type(func) == "function" then
			fp:write(name, " ... ")
			ok, err = optcall("beforeEach")
			if ok then ok, err = optcall(name) end
			if ok then ok, err = optcall("afterEach") end
			if ok then
				fp:write("\27[32mPASS\27[0m\n")
				passed = passed + 1
			else
				fp:write("\27[31mFAIL\27[0m\n")
				failed = failed + 1
				errors[name] = err
			end
		end
	end
	local ok, err = optcall("afterAll")
	if not ok then
		failed = failed + 1
		errors.afterAll = err
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

function driver:runscript(script, errhdlr, fp)
	assert(type(script) == "string")
	errhdlr = errhdlr or debug.traceback
	fp = fp or io.stderr
	return self:_runscript(script, errhdlr, fp)
end

function driver:_runscript(script, errhdlr, fp)
	local module = require(script)
	assert(type(module) == "table")
	fp:write('Test script: ', script, '\n')
	fp:write(string.rep('=', 80), '\n')
	local failed, passed, errors = driver:runmodule(module, errhdlr)
	fp:write(string.rep('=', 80), '\n')
	local haserror = false
	for name, err in pairs(errors) do
		if haserror then
			fp:write(string.rep('-', 80), '\n')
		end
		fp:write("Error on ", name, ":\n", tostring(err), '\n')
		haserror = true
	end
	if haserror then
		fp:write(string.rep('=', 80), '\n')
	end
	if failed ~= 0 then
		return false, failed .. " tests failed"
	end
	driver:collectallgarbage()
	local list = memdb.get_amb_list()
	if #list ~= 0 then
		for i, v in ipairs(list) do
			fp:write(v.address, ' (', v.size, ' B) @ ', v.filename, ':', v.line , ' (', v.funcname, ')\n')
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
