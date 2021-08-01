local lt = require "lt"
local memdb = require "memdb"

local t = { size = 1, afterAll = memdb.afterAll }

function t:testLogChannels()
	local channels = memdb.get_log_channel_list()
	for _, channel in ipairs(channels) do
		lt.assertType(channel, "string");
		local val = memdb.get_log_channel(channel)
		lt.assertType(val, "boolean");
		memdb.set_log_channel(channel, not val)
		lt.assertEqual(memdb.get_log_channel(channel), not val)
		memdb.set_log_channel(channel, val)
		lt.assertEqual(memdb.get_log_channel(channel), val)
	end
end

function t:testAllLogChannels()
	local channels = memdb.get_log_channel_list()
	memdb.set_all_log_channels(true)
	for _, channel in ipairs(channels) do
		lt.assertTrue(memdb.get_log_channel(channel))
	end
	memdb.set_all_log_channels(false)
	for _, channel in ipairs(channels) do
		lt.assertFalse(memdb.get_log_channel(channel))
	end
end

function t:nextSize()
	local size = t.size
	if t.size < 8 then
		t.size = t.size * 2
	else
		t.size = t.size + 8
	end
	return size
end

function t:getAMBbySize(size)
	for _, amb in ipairs(memdb.get_amb_list()) do
		if amb.size == size then
			return amb
		end
	end
	return nil
end

function t:testUnsafePointer()
	local size = self:nextSize()
	lt.assertNil(self:getAMBbySize(size))
	local p = memdb.malloc(size)
	local lst = memdb.get_amb_list()
	local amb = self:getAMBbySize(size)
	lt.assertNotNil(amb)
	lt.assertEqual(amb.funcname, 'malloc')
	lt.assertTrue(memdb.free(p))
	lt.assertFalse(memdb.free(p))
	lt.assertNil(self:getAMBbySize(size))
end

function t:testSafePointer()
	local size = self:nextSize()
	lt.assertNil(self:getAMBbySize(size))
	local p = memdb.safe_malloc(size)
	local amb = self:getAMBbySize(size)
	lt.assertNotNil(amb)
	lt.assertEqual(amb.funcname, 'malloc')
	lt.assertFalse(pcall(memdb.free, p))
	p = nil
	collectgarbage()
	lt.assertNil(self:getAMBbySize(size))
end

return t
