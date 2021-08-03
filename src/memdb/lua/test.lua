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

function t:testSetGetFailCountdown()
	for i, value in ipairs{0, 1, 2, 3, 0} do
		memdb.set_fail_countdown(value)
		lt.assertEqual(memdb.get_fail_countdown(), value, i)
	end
end

function t:testFailMalloc()
	memdb.set_fail_countdown(2)
	lt.assertEqual(memdb.get_fail_countdown(), 2)
	self:testSafePointer()
	lt.assertEqual(memdb.get_fail_countdown(), 1)
	lt.assertSubstring('bad malloc', lt.assertRaises(self.testSafePointer, self))
	lt.assertEqual(memdb.get_fail_countdown(), 1)
	lt.assertSubstring('bad malloc', lt.assertRaises(self.testSafePointer, self))
	memdb.set_fail_countdown(0)
	lt.assertEqual(memdb.get_fail_countdown(), 0)
	self:testSafePointer()
	lt.assertEqual(memdb.get_fail_countdown(), 0)
end

function t:testCountMallocs()
	lt.assertSubstring('function', lt.assertRaises(memdb.count_mallocs, 123))
	lt.assertEqual(0, memdb.count_mallocs(function() end))
	lt.assertEqual(1, memdb.count_mallocs(function() memdb.safe_malloc(1) end))
	lt.assertEqual(1000, memdb.count_mallocs(function()
		for i = 1, 1000 do memdb.safe_malloc(1) end
	end))
	lt.assertSubstring('integer overflow',
		lt.assertRaises(memdb.count_mallocs, function()
			memdb.set_fail_countdown(0)
		end))
end

return t
