local argvp = require "argvp"
local lt = require "lt"

local t = { afterAll = argvp.memdb.afterAll }

function t:testEmpty()
	local p = argvp.ArgumentParser{}
end

function t:testOne()
	local p = argvp.ArgumentParser{'abc'}
end

function t:testMultiple()
	local p = argvp.ArgumentParser{'abc', 'def', 'ghi'}
end

return t
