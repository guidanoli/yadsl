local driver = require "lt.driver"

local testscripts = {
	"lt.tests.test",
	"memdb.lua.test",
	"diff.lua.test",
	"string.lua.test",
	"argvp.lua.test",
}

local errors = 0
local before = os.clock()
for _, testscript in pairs(testscripts) do
	local ok, err = driver:runscript(testscript)
	if not ok then
		io.stderr:write(driver:center('summary', 80, '-'), '\n', tostring(err), '\n')
		errors = errors + 1
	end
end
local after = os.clock()
local dt = after - before
local dtmsg = string.format('(%g s)', dt)
local errmsg = errors .. ' error' .. ((errors == 1) and '' or 's')
io.stderr:write(driver:center(errmsg .. ' ' .. dtmsg, 80, '='), '\n')
if errors ~= 0 then
	os.exit(1)
end
