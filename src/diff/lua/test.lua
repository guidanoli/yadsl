local t = {}
local lt = require "lt"
local m = require "diff"

function t:testEqualStrings()
	lt.assertEqual(m.diff("", ""), 0)
	lt.assertEqual(m.diff(" ", " "), 0)
	lt.assertEqual(m.diff("algo", "algo"), 0)
	lt.assertEqual(m.diff("algo assim", "algo assim"), 0)
end

function t:testUnequalStrings()
	lt.assertNotEqual(m.diff("a", ""), 0)
	lt.assertNotEqual(m.diff("a", " "), 0)
	lt.assertNotEqual(m.diff("abc", "abd"), 0)
	lt.assertNotEqual(m.diff("space", "s p a c e"), 0)
	lt.assertNotEqual(m.diff("lspace", " lspace"), 0)
	lt.assertNotEqual(m.diff("rspace", "rspace "), 0)
	lt.assertNotEqual(m.diff("case-sensitive", "CaSe-SeNsitIvE"), 0)
end

return t
