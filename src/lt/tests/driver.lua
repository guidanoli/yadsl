local realdriver = require "lt.driver"

local driver = {}

function driver:testmodule(module, expected_errors)
	assert(type(module) == "table")
	assert(type(expected_errors) == "table")
	for funcname, exp_error in pairs(expected_errors) do
		assert(type(funcname) == "string", funcname)
		assert(type(exp_error) == "string", exp_error)
	end
	return self:_testmodule(module, expected_errors)
end

function driver:testscript(scriptname)
	assert(type(scriptname) == "string")
	return self:_testscript(scriptname)
end

function driver:_testmodule(module, expected_errors)
	local function errhdlr(err) return err end
	local tmpfp = io.tmpfile()
	local failed, passed, errors = realdriver:runmodule(module, errhdlr, tmpfp)
	tmpfp:close()
	for k, v in pairs(errors) do
		local w = expected_errors[k]
		assert(type(v) == 'string')
		assert(type(w) == 'string')
		if not v:find(w) then
			return false, "error on function " .. tostring(k) .. " not caught"
		end
	end
	for k, w in pairs(expected_errors) do
		local v = errors[k]
		assert(type(v) == 'string')
		assert(type(w) == 'string')
		if not v:find(w) then
			return false, "error on function " .. tostring(k) .. " not raised"
		end
	end
	return true
end

function driver:_testscript(scriptname)
	local ok, ret = pcall(require, scriptname)
	if not ok then
		return false, ret
	end
	local module = ret
	local errors = ret.errors or {}
	return self:testmodule(module, errors)
end

return driver
