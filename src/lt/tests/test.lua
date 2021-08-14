local driver = require "lt.tests.driver"

local t = {}

local testscripts = {
	"before_and_after",
	"check_param",
	"empty",
	"failing_test",
	"function_name",
	"lt_lib",
	"return_array",
}

for _, testscript in ipairs(testscripts) do
	t['test_'..testscript] = function()
		assert(driver:testscript("lt.tests." .. testscript))
	end
end

return t
