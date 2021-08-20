-------------------------------------------------
-- Lua test driver
-- ===============
--
-- A driver for running Lua test benches.
--
-- Usage as a Lua script
-- =====================
--
-- > lua luatester/driver.lua <script>
--
-------------------------------------------------

local Driver = {}

---------------------------
-- Public functions
---------------------------

-- Run test bench table
-- Arguments:
--   tb   - table of functions        $string
-- Returns:
--   [1]  - number of test cases      $number
function Driver:runTestBench(tb)
	assert(type(tb) == "table", "expected test bench to be table")
	return self:_runTestBench(tb)
end

---------------------------
-- Private functions
---------------------------

function Driver:_doMethod(tb, func)
	if func ~= nil then
		local ok, err = xpcall(func, debug.traceback, tb)
		if not ok then
			error(err, 0)
		end
	end
end

function Driver:_doTestCase(tb, func, before, after)
	self:_doMethod(tb, before)
	local ok, err = pcall(self._doMethod, self, tb, func)
	self:_doMethod(tb, after)
	if not ok then
		error(err, 0)
	end

end

function Driver:_runTestBench(tb)
	local beforeAll = tb.beforeAll
	local beforeEach = tb.beforeEach
	local afterEach = tb.afterEach
	local afterAll = tb.afterAll

	self:_doMethod(tb, beforeAll)

	local ok = true
	local err
	local nfuncs = 0

	for name, func in pairs(tb) do
		if type(name) == "string" and name:find("^test") then
			io.stderr:write(name, " ... ")
			ok, err = pcall(self._doTestCase, self, tb, func, beforeEach, afterEach)
			if ok then
				io.stderr:write("\27[32mPASS\27[0m\n")
				nfuncs = nfuncs + 1
			else
				io.stderr:write("\27[31mFAIL\27[0m\n")
				break
			end
		end
	end
	
	self:_doMethod(tb, afterAll)

	if not ok then
		error(err, 0)
	end

	return nfuncs
end

---------------------------
-- Driver as script
---------------------------

if type(arg) == "table" and arg[0]:find("driver%.lua$") then
	local scriptname = assert(arg[1], "missing test script")
	local testbench = require(scriptname)
	Driver:runTestBench(testbench)
end

---------------------------
-- Driver as module
---------------------------

return Driver
