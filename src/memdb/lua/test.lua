local lt = require "lt"
local memdb = require "memdb"

local t = {}

function t:assertNotNull(p, ...)
	lt.assertNotEqual(p, memdb.nullptr(), ...)
end

function t:assertNull(p, ...)
	lt.assertEqual(p, memdb.nullptr(), ...)
end

function t:testRawMallocGC()
	memdb.rawmalloc(1)
end

function t:testMallocGC()
	memdb.malloc(1, 'abc', 123)
end

function t:testNullPtrGC()
	memdb.nullptr()
end

function t:testRawMallocRawFree()
	memdb.rawmalloc(1):rawfree()
end

function t:testRawMallocFree()
	memdb.rawmalloc(1):free('abc', 345)
end

function t:testMallocRawFree()
	memdb.malloc(1, 'abc', 345):rawfree()
end

function t:testMallocFree()
	memdb.malloc(1, 'abc', 345):free('cde', 678)
end

function t:testNullPtrRawFree()
	memdb.nullptr():rawfree()
end

function t:testNullPtrFree()
	memdb.nullptr():free('abc', 234)
end

function t:testNullPtrReallocGC()
	memdb.nullptr():realloc(1, 'abc', 123)
end

function t:testNullPtrRawReallocGC()
	memdb.nullptr():rawrealloc(1)
end

function t:testNullPtrReallocFree()
	local p = memdb.nullptr()
	lt.assertTrue(p:realloc(1, 'abc', 123))
	p:free('cde', 456)
end

function t:testNullPtrRawReallocFree()
	local p = memdb.nullptr()
	lt.assertTrue(p:rawrealloc(1))
	p:free('cde', 456)
end

function t:testNullPtrReallocRawFree()
	local p = memdb.nullptr()
	lt.assertTrue(p:realloc(1, 'abc', 123))
	p:rawfree()
end

function t:testNullPtrRawReallocRawFree()
	local p = memdb.nullptr()
	lt.assertTrue(p:rawrealloc(1))
	p:rawfree()
end

function t:testMallocReallocGrow()
	local p = memdb.malloc(2, 'abc', 123)
	lt.assertTrue(p:realloc(4, 'cde', 456))
end

function t:testMallocReallocShrink()
	local p = memdb.malloc(2, 'abc', 123)
	lt.assertTrue(p:realloc(1, 'cde', 456))
end

function t:testMallocRawReallocGrow()
	local p = memdb.malloc(2, 'abc', 123)
	lt.assertTrue(p:rawrealloc(4))
end

function t:testMallocRawReallocShrink()
	local p = memdb.malloc(2, 'abc', 123)
	lt.assertTrue(p:rawrealloc(1))
end

function t:testRawMallocReallocGrow()
	local p = memdb.rawmalloc(2)
	lt.assertTrue(p:realloc(4, 'cde', 456))
end

function t:testRawMallocReallocShrink()
	local p = memdb.rawmalloc(2)
	lt.assertTrue(p:realloc(1, 'cde', 456))
end

function t:testRawMallocRawReallocGrow()
	local p = memdb.rawmalloc(2)
	lt.assertTrue(p:rawrealloc(4))
end

function t:testRawMallocRawReallocShrink()
	local p = memdb.rawmalloc(2)
	lt.assertTrue(p:rawrealloc(1))
end

function t:testEqualNullPtr()
	local p1 = memdb.nullptr()
	local p2 = memdb.nullptr()
	lt.assertRawNotEqual(p1, p2)
	lt.assertEqual(p1, p2)
end

function t:testNotNull()
	local t = {
		memdb.malloc(1, 'abc', 345),
		memdb.rawmalloc(1),
		memdb.calloc(1, 1, 'abc', 345),
		memdb.rawcalloc(1, 1),
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

function t:testBadMalloc()
	lt.assertSubstring('bad malloc', lt.assertRaises(memdb.malloc, math.maxinteger, 'abc', 234))
	lt.assertSubstring('bad malloc', lt.assertRaises(memdb.rawmalloc, math.maxinteger))
	lt.assertSubstring('bad malloc', lt.assertRaises(memdb.calloc, math.maxinteger, math.maxinteger, 'abc', 234))
	lt.assertSubstring('bad malloc', lt.assertRaises(memdb.rawcalloc, math.maxinteger, math.maxinteger))
	local nullptr = memdb.nullptr()
	lt.assertFalse(nullptr:realloc(math.maxinteger, 'abc', 234))
	lt.assertFalse(nullptr:rawrealloc(math.maxinteger))
end

return t
