local driver = require "lt.tests.driver"

-- All tests use lt
lt = require "lt"

local testscripts = {
	"before_and_after",
	"check_param",
	"empty",
	"failing_test",
	"function_name",
	"lt_lib",
	"return_array",
}

local errors = 0
for _, testscript in pairs(testscripts) do
	testscript = "lt.tests." .. testscript
	local ok, err = driver:testscript(testscript)
	if not ok then
		io.stderr:write(testscript, ': ', err, '\n')
		errors = errors + 1
	end
end
if errors > 0 then
	os.exit(1)
end
