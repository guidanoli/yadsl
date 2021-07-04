local lt = require "lt"
local m = require "stringutils"

local t = { afterAll = m.memdb.afterAll }

-- [a-zA-Z0-9]
t.charset = {}
for i = 48,  57 do table.insert(t.charset, string.char(i)) end
for i = 65,  90 do table.insert(t.charset, string.char(i)) end
for i = 97, 122 do table.insert(t.charset, string.char(i)) end

function t:testDuplicate()
	local strings = {
		"",
		"a",
		"abc",
		"AbC",
		string.rep('a', 127),
	}
	for _, str in pairs(strings) do
		local dup = m.duplicate(str)
		lt.assertEqual(str, dup)
	end
end

function t:generateString()
	local MAX_SIZE = 1000
	local size = math.random(MAX_SIZE)
	local str = ''
	for i = 1, size do
		str = str .. self.charset[math.random(#self.charset)]
	end
	return str
end

function t:testDuplicateMetamorphic()
	math.randomseed(os.time())
	local TEST_CASES = 1000
	for TEST_CASE_INDEX = 1, TEST_CASES do
		local str = self:generateString()
		local dup = m.duplicate(str)
		lt.assertEqual(str, dup)
	end
end

function t:testCompareIgnoringCase()
	lt.assertEqual(m.compare_ic('', ''), 0)
	lt.assertEqual(m.compare_ic(' ', ' '), 0)
	lt.assertEqual(m.compare_ic('a', 'a'), 0)
	lt.assertEqual(m.compare_ic('A', 'a'), 0)
	lt.assertEqual(m.compare_ic('a', 'A'), 0)
	lt.assertEqual(m.compare_ic('A', 'A'), 0)
	lt.assertGreaterThan(m.compare_ic('b', 'a'), 0)
	lt.assertGreaterThan(m.compare_ic('b', 'A'), 0)
	lt.assertGreaterThan(m.compare_ic('B', 'a'), 0)
	lt.assertGreaterThan(m.compare_ic('B', 'A'), 0)
	lt.assertLessThan(m.compare_ic('a', 'b'), 0)
	lt.assertLessThan(m.compare_ic('a', 'B'), 0)
	lt.assertLessThan(m.compare_ic('A', 'b'), 0)
	lt.assertLessThan(m.compare_ic('A', 'B'), 0)
end

function t:testCompareIgnoreCaseMetamorphic()
	math.randomseed(os.time())
	local TEST_CASES = 1000
	for TEST_CASE_INDEX = 1, TEST_CASES do
		local str = self:generateString()
		lt.assertEqual(m.compare_ic(str, str), 0)
		local lstr = string.lower(str)
		local ustr = string.upper(str)
		lt.assertEqual(m.compare_ic(str, lstr), 0)
		lt.assertEqual(m.compare_ic(str, ustr), 0)
		lt.assertEqual(m.compare_ic(lstr, ustr), 0)
	end
end

return t
