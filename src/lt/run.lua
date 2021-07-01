local driver = require "lt.driver"

local testscripts = {
	"diff.lua.test",
}

local errors = 0
for _, testscript in pairs(testscripts) do
	local ok, err = driver:runscript(testscript)
	if not ok then
		io.stderr:write(err, '\n')
		errors = errors + 1
	end
end
if errors > 0 then
	os.exit(1)
end
