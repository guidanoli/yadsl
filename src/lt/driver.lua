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
			self:collectallgarbage()
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

function driver:center(title, size, decorator)
	assert(type(title) == 'string')
	assert(type(size) == 'number')
	assert(type(decorator) == 'string')
	return self:_center(title, size, decorator)
end

function driver:_center(title, size, decorator)
	local dec_total = size - string.len(title) - 2
	local dec_left = dec_total // 2
	local dec_right = dec_total - dec_left
	return string.rep(decorator, dec_left) .. ' ' .. title .. ' ' .. string.rep(decorator, dec_right)
end

function driver:_runscript(script, errhdlr, fp)
	local module = require(script)
	assert(type(module) == "table")
	fp:write(self:center(script, 80, '='), '\n')
	local failed, passed, errors = driver:runmodule(module, errhdlr)
	for name, err in pairs(errors) do
		fp:write(self:center('error on ' .. name, 80, '-'), '\n', tostring(err), '\n')
	end
	if failed ~= 0 then
		return false, failed .. " tests failed"
	end
	return true
end

if type(arg) == "table" and arg[0]:find("driver%.lua$") then
	local script = assert(arg[1], "missing test script")
	assert(driver:runscript(script))
end

return driver
