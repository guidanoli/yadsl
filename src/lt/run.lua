local driver = require "lt.driver"

local testscripts = {
	"diff.lua.test",
	"memdb.lua.test",
	"lt.tests.test",
}

local errors = 0
for _, testscript in pairs(testscripts) do
	local ok, err = driver:runscript(testscript)
	if not ok then
		io.stderr:write(driver:center('summary', 80, '-'), '\n', tostring(err), '\n')
		errors = errors + 1
	end
end
if errors > 0 then
	os.exit(1)
end
