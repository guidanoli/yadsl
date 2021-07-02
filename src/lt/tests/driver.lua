local realdriver = require "lt.driver"

local driver = {}

function driver:testmodule(module, expected_failed, expected_passed, expected_errors)
	assert(type(module) == "table")
	assert(type(expected_passed) == "number")
	assert(type(expected_failed) == "number")
	assert(type(expected_errors) == "table")
	return self:_testmodule(module, expected_failed, expected_passed, expected_errors)
end

function driver:testscript(scriptname)
	assert(type(scriptname) == "string")
	return self:_testscript(scriptname)
end

function driver:shallowEqual(t1, t2)
	assert(type(t1) == "table")
	assert(type(t2) == "table")
	return self:_shallowEqual(t1, t2)
end

function driver:_testmodule(module, expected_failed, expected_passed, expected_errors)
	local function errhdlr(err) return err end
	local tmpfp = io.tmpfile()
	local failed, passed, errors = realdriver:runmodule(module, errhdlr, tmpfp)
	tmpfp:close()
	if failed ~= expected_failed then
		return false, "failed = " .. failed
	elseif passed ~= expected_passed then
		return false, "passed = " .. passed
	else
		for k, v in pairs(errors) do
			local w = expected_errors[k]
			assert(type(v) == 'string')
			if not v:find(w) then
				return false, "errors differ by key " .. tostring(key)
			end
		end
		return true
	end
end

function driver:_testscript(scriptname)
	local ok, ret = pcall(require, scriptname)
	if not ok then
		return false, ret
	end
	local module = ret
	local failed = ret.failed or 0
	local passed = ret.passed or 0
	local errors = ret.errors or {}
	return self:testmodule(module, failed, passed, errors)
end

return driver
