local t = {}
local lt = require "lt"
local diff = require "diff"

function t:testEqualStrings()
	lt.assertEqual(diff.diff("", ""), 0)
	lt.assertEqual(diff.diff(" ", " "), 0)
	lt.assertEqual(diff.diff("algo", "algo"), 0)
	lt.assertEqual(diff.diff("algo assim", "algo assim"), 0)
end

function t:testUnequalStrings()
	lt.assertNotEqual(diff.diff("a", ""), 0)
	lt.assertNotEqual(diff.diff("a", " "), 0)
	lt.assertNotEqual(diff.diff("abc", "abd"), 0)
	lt.assertNotEqual(diff.diff("space", "s p a c e"), 0)
	lt.assertNotEqual(diff.diff("lspace", " lspace"), 0)
	lt.assertNotEqual(diff.diff("rspace", "rspace "), 0)
	lt.assertNotEqual(diff.diff("case-sensitive", "CaSe-SeNsitIvE"), 0)
end

function t:afterAll()
	lt.assertEqual(diff.memdb.get_amb_list_size(), 0)
end

return t
