-------------------------------------------------
-- Lua test runner
-- ===============
--
-- A script for running all Lua tests.
--
-- Usage as a Lua script
-- =====================
--
-- > lua luatester/run.lua
--
-------------------------------------------------

local Driver = require "luatester.driver"

local Run = {}

function Run:centerText(title, size, char)
	assert(type(char) == 'string' and #char == 1)
	if #title > 0 then title = ' ' .. title .. ' ' end
	local dec_total = size - #title
	local dec_left = dec_total // 2
	local dec_right = dec_total - dec_left
	return string.rep(char, dec_left) .. title .. string.rep(char, dec_right)
end

function Run:timeIt(func)
	local start = os.clock()
	func()
	return os.clock() - start
end

function Run:runTestScripts(testScripts)
	local passed = 0
	local failed = 0
	local time = self:timeIt(function()
		for _, testScript in pairs(testScripts) do
			io.stderr:write(self:centerText(testScript , 80, '='), '\n')
			local testBench = require(testScript)
			local ok, err = pcall(Driver.runTestBench, Driver, testBench)
			if ok then
				passed = passed + 1
			else
				io.stderr:write(self:centerText('error', 80, '-'), '\n')
				io.stderr:write(tostring(err), '\n')
				failed = failed + 1
			end
		end
	end)
	local summaryMessage = string.format('%d failed, %d passed (%g s)', failed, passed, time)
	io.stderr:write(self:centerText(summaryMessage , 80, '='), '\n')
	return failed, passed
end

---------------------------
-- Run as script
---------------------------

if type(arg) == "table" and arg[0]:find("run%.lua$") then
	local testScripts = require "luatester.tests"
	local failed = Run:runTestScripts(testScripts)
	os.exit(failed == 0)
end

---------------------------
-- Run as module
---------------------------

return Run
