local lt = require "lt"
local memdb = require "memdb"

local t = {}

function t:assertNotNull(p, ...)
	lt.assertNotEqual(p, memdb.nullptr(), ...)
end

function t:assertNull(p, ...)
	lt.assertEqual(p, memdb.nullptr(), ...)
end

function t:testMallocGC()
	memdb.malloc(1, 'abc', 123)
end

function t:testNullPtrGC()
	memdb.nullptr()
end

function t:testNullPtrFree()
	memdb.nullptr():free('abc', 234)
end

function t:testNullPtrReallocGC()
	memdb.nullptr():realloc(1, 'abc', 123)
end

function t:testNullPtrReallocFree()
	local p = memdb.nullptr()
	p:realloc(1, 'abc', 123)
	p:free('cde', 456)
end

function t:testMallocReallocGrow()
	local p = memdb.malloc(2, 'abc', 123)
	p:realloc(4, 'cde', 456)
end

function t:testMallocReallocShrink()
	local p = memdb.malloc(2, 'abc', 123)
	p:realloc(1, 'cde', 456)
end

function t:testEqualNullPtr()
	local p1 = memdb.nullptr()
	local p2 = memdb.nullptr()
	lt.assertNotRawEqual(p1, p2)
	lt.assertEqual(p1, p2)
end

function t:testNotNull()
	local t = {
		memdb.malloc(1, 'abc', 345),
		memdb.calloc(1, 1, 'abc', 345),
		memdb.nullptr():realloc(1, 'abc', 345),
		memdb.malloc(1, 'cde', 234):realloc(1, 'abc', 345),
	}

	for pointer in ipairs(t) do
		local found_itself = false
		self:assertNotNull(pointer)
		for other_pointer in ipairs(t) do
			if not rawequal(pointer, other_pointer) then
				lt.assertNotEqual(pointer, other_pointer)
			else
				lt.assertFalse(found_itself)
				found_itself = true;
			end
		end
		lt.assertTrue(found_itself)
	end
end

function t:testTooLargeAllocationSize()
	lt.assertSubstring('bad malloc', lt.assertRaises(memdb.malloc, math.maxinteger, 'abc', 234))
	lt.assertSubstring('bad calloc', lt.assertRaises(memdb.calloc, math.maxinteger, math.maxinteger, 'abc', 234))
	local ptr = memdb.nullptr()
	lt.assertSubstring('bad realloc', lt.assertRaises(ptr.realloc, ptr, math.maxinteger, 'abc', 234))
end

function t:testToConst()
	for _, pointer in ipairs{ memdb.nullptr(), memdb.malloc(1, 'abc', 123) } do
		local cte1 = pointer:asconst()
		local cte2 = pointer:asconst()
		lt.assertNotRawEqual(cte1, cte2)
		lt.assertEqual(cte1, cte2)
	end
end

function t:testFailAllocation()
	local file = 'abc'
	local line = 123
	local ptr = memdb.malloc(1, 'cde', 345)
	for _, case in ipairs{
		{ args = { memdb.malloc, 8 }, msg = 'bad malloc', event = { size = 8, func = 'malloc' } },
		{ args = { memdb.calloc, 8, 4 }, msg = 'bad calloc', event = { nmemb = 8, size = 4, func = 'calloc' } },
		{ args = { ptr.realloc, ptr, 8 }, msg = 'bad realloc', event = { ptr = ptr:asconst(), size = 8, func = 'realloc' } },
	} do
		local event = nil
		local levent = nil
		local constptr = nil
		table.insert(case.args, file)
		table.insert(case.args, line)
		local ok, err = memdb.pcall{
			ask_cb = function(e)
				event = e
				return false
			end,
			listen_cb = function(e, p)
				if e.func ~= 'free' then
					levent = e
					constptr = p
				end
			end,
			table.unpack(case.args),
		}
		lt.assertFalse(ok)
		lt.assertNotNil(event)
		lt.assertNotNil(levent)
		lt.assertNotNil(constptr)
		lt.assertSubstring(case.msg, err)
		lt.assertEqual(event.file, file)
		lt.assertEqual(event.line, line)
		lt.assertDeepEqual(event, levent)
		lt.assertEqual(constptr, memdb.nullptr():asconst())
	end
end

function t:testListen()
	local file = 'abc'
	local line = 123
	local ptr = memdb.malloc(1, 'cde', 345)
	local ptr2 = memdb.malloc(1, 'cde', 345)
	for _, case in ipairs{
		{ args = { memdb.malloc, 8 }, event = { size = 8, func = 'malloc' } },
		{ args = { memdb.calloc, 8, 4 }, event = { nmemb = 8, size = 4, func = 'calloc' } },
		{ args = { ptr.realloc, ptr, 8 }, event = { ptr = ptr:asconst(), size = 8, func = 'realloc' } },
		{ args = { ptr2.free, ptr2 }, event = { ptr = ptr2:asconst(), func = 'free' } },
	} do
		local event = nil
		local constptr = nil
		table.insert(case.args, file)
		table.insert(case.args, line)
		local ok, ret = memdb.pcall{
			listen_cb = function(e, p)
				event = e
				constptr = p
			end,
			table.unpack(case.args),
		}
		lt.assertTrue(ok)
		lt.assertNotNil(event)
		lt.assertNotNil(constptr)
		lt.assertEqual(event.file, file)
		lt.assertEqual(event.line, line)
		if ret ~= nil then
			lt.assertEqual(ret:asconst(), constptr)
		end
	end
end

function t:testPcallManyArgs()
	local t = {
		function(...)
			local args = table.pack(...)
			return #args
		end
	}
	for i = 1, 100 do
		t[i+1] = 123
	end
	local ok, ret = memdb.pcall(t)
	lt.assertTrue(ok)
	lt.assertEqual(ret, 100)
end

function t:testPcallManyReturns()
	local t = {
		function(...)
			local args = table.pack(...)
			for k, v in pairs(args) do
				args[k] = v + 1
			end
			return table.unpack(args)
		end
	}
	for i = 1, 100 do
		t[i+1] = i
	end
	local ret = table.pack(memdb.pcall(t))
	lt.assertTrue(ret[1])
	for i = 1, 100 do
		lt.assertEqual(ret[i+1], i+1)
	end
	lt.assertNil(ret[102])
end

function t:testPcallBasicPaths()
	-- no function
	lt.assertSubstring('missing function', lt.assertRaises(memdb.pcall, {}))
	-- only function
	do
		local num = math.random()
		local ok, ret = memdb.pcall{function() error(num) end}
		lt.assertFalse(ok)
		lt.assertSubstring(ret, num)
	end
	do
		local num = math.random()
		local ok, ret = memdb.pcall{function() return num end}
		lt.assertTrue(ok)
		lt.assertEqual(ret, num)
	end
	-- function + argument
	do
		local num = math.random()
		local ok, ret = memdb.pcall{function(n) error(n^2) end, num}
		lt.assertFalse(ok)
		lt.assertSubstring(ret, num^2)
	end
	do
		local num = math.random()
		local ok, ret = memdb.pcall{function(n) return n^2 end, num}
		lt.assertTrue(ok)
		lt.assertEqual(ret, num^2)
	end
	-- function + listen_cb
	do
		local event, constptr
		local ok, ret = memdb.pcall{
			memdb.malloc, 8, 'abc', 123,
			listen_cb = function(e, p)
				event = e
				constptr = p
			end,
		}
		lt.assertTrue(ok)
		self:assertNotNull(ret)
		lt.assertEqual(ret:asconst(), constptr)
		lt.assertDeepEqual(event, {
			func = 'malloc',
			file = 'abc',
			line = 123,
			size = 8,
		})
	end
	-- function + listen_cb raising error
	do
		local event, constptr
		local ok, ret = memdb.pcall{
			memdb.malloc, 8, 'abc', 123,
			listen_cb = function(e, p)
				event = e
				constptr = p
				error('etwas')
			end,
		}
		lt.assertTrue(ok)
		self:assertNotNull(ret)
		lt.assertEqual(ret:asconst(), constptr)
		lt.assertDeepEqual(event, {
			func = 'malloc',
			file = 'abc',
			line = 123,
			size = 8,
		})
	end
	-- function + ask_cb raising error
	do
		local event
		local ok, ret = memdb.pcall{
			memdb.malloc, 8, 'abc', 123,
			ask_cb = function(e)
				event = e
				error('etwas')
			end,
		}
		lt.assertFalse(ok)
		lt.assertSubstring(ret, 'bad malloc')
		lt.assertDeepEqual(event, {
			func = 'malloc',
			file = 'abc',
			line = 123,
			size = 8,
		})
	end
	-- function + ask_cb returning true
	do
		local event
		local ok, ret = memdb.pcall{
			memdb.malloc, 8, 'abc', 123,
			ask_cb = function(e)
				event = e
				return true
			end,
		}
		lt.assertTrue(ok)
		self:assertNotNull(ret)
		lt.assertDeepEqual(event, {
			func = 'malloc',
			file = 'abc',
			line = 123,
			size = 8,
		})
	end
	-- function + ask_cb returning false
	do
		local event
		local ok, ret = memdb.pcall{
			memdb.malloc, 8, 'abc', 123,
			ask_cb = function(e)
				event = e
				return false
			end,
		}
		lt.assertFalse(ok)
		lt.assertSubstring(ret, 'bad malloc')
		lt.assertDeepEqual(event, {
			func = 'malloc',
			file = 'abc',
			line = 123,
			size = 8,
		})
	end
	-- function + listen_cb + ask_cb raising error
	do
		local levent, event, constptr
		local ok, ret = memdb.pcall{
			memdb.malloc, 8, 'abc', 123,
			listen_cb = function(e, p)
				levent = e
				constptr = p
			end,
			ask_cb = function(e)
				event = e
				error('etwas')
			end,
		}
		lt.assertFalse(ok)
		lt.assertSubstring(ret, 'bad malloc')
		lt.assertDeepEqual(event, levent)
		lt.assertDeepEqual(levent, {
			func = 'malloc',
			file = 'abc',
			line = 123,
			size = 8,
		})
	end
	-- function + listen_cb + ask_cb returning true
	do
		local levent, event, constptr
		local ok, ret = memdb.pcall{
			memdb.malloc, 8, 'abc', 123,
			listen_cb = function(e, p)
				levent = e
				constptr = p
			end,
			ask_cb = function(e)
				event = e
				return true
			end,
		}
		lt.assertTrue(ok)
		self:assertNotNull(ret)
		lt.assertEqual(ret:asconst(), constptr)
		lt.assertDeepEqual(event, levent)
		lt.assertDeepEqual(event, {
			func = 'malloc',
			file = 'abc',
			line = 123,
			size = 8,
		})
	end
	-- function + listen_cb + ask_cb returning false
	do
		local levent, event, constptr
		local ok, ret = memdb.pcall{
			memdb.malloc, 8, 'abc', 123,
			listen_cb = function(e, p)
				levent = e
				constptr = p
			end,
			ask_cb = function(e)
				event = e
				return false
			end,
		}
		lt.assertFalse(ok)
		lt.assertSubstring(ret, 'bad malloc')
		lt.assertDeepEqual(event, levent)
		lt.assertDeepEqual(levent, {
			func = 'malloc',
			file = 'abc',
			line = 123,
			size = 8,
		})
	end
	-- function + listen_cb raising error + ask_cb raising error
	do
		local levent, event, constptr
		local ok, ret = memdb.pcall{
			memdb.malloc, 8, 'abc', 123,
			listen_cb = function(e, p)
				levent = e
				constptr = p
				error('etwas')
			end,
			ask_cb = function(e)
				event = e
				error('etwas')
			end,
		}
		lt.assertFalse(ok)
		lt.assertSubstring(ret, 'bad malloc')
		lt.assertDeepEqual(event, levent)
		lt.assertDeepEqual(levent, {
			func = 'malloc',
			file = 'abc',
			line = 123,
			size = 8,
		})
	end
	-- function + listen_cb raising error + ask_cb returning true
	do
		local levent, event, constptr
		local ok, ret = memdb.pcall{
			memdb.malloc, 8, 'abc', 123,
			listen_cb = function(e, p)
				levent = e
				constptr = p
				error('etwas')
			end,
			ask_cb = function(e)
				event = e
				return true
			end,
		}
		lt.assertTrue(ok)
		self:assertNotNull(ret)
		lt.assertEqual(ret:asconst(), constptr)
		lt.assertDeepEqual(event, levent)
		lt.assertDeepEqual(event, {
			func = 'malloc',
			file = 'abc',
			line = 123,
			size = 8,
		})
	end
	-- function + listen_cb raising error + ask_cb returning false
	do
		local levent, event, constptr
		local ok, ret = memdb.pcall{
			memdb.malloc, 8, 'abc', 123,
			listen_cb = function(e, p)
				levent = e
				constptr = p
				error('etwas')
			end,
			ask_cb = function(e)
				event = e
				return false
			end,
		}
		lt.assertFalse(ok)
		lt.assertSubstring(ret, 'bad malloc')
		lt.assertDeepEqual(event, levent)
		lt.assertDeepEqual(levent, {
			func = 'malloc',
			file = 'abc',
			line = 123,
			size = 8,
		})
	end
end

return t
