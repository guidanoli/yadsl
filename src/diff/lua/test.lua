local lt = require "lt"
local diff = require "diff"

local t = { afterAll = diff.memdb.afterAll }

function t:testEqualStrings()
	lt.assertEqual(diff.diff("", ""), 0)
	lt.assertEqual(diff.diff(" ", " "), 0)
	lt.assertEqual(diff.diff("algo", "algo"), 0)
	lt.assertEqual(diff.diff("algo assim", "algo assim"), 0)
end

function t:testUnequalStrings()
	lt.assertGreaterThan(diff.diff("a", ""), 0)
	lt.assertGreaterThan(diff.diff("a", " "), 0)
	lt.assertGreaterThan(diff.diff("abc", "abd"), 0)
	lt.assertGreaterThan(diff.diff("space", "s p a c e"), 0)
	lt.assertGreaterThan(diff.diff("lspace", " lspace"), 0)
	lt.assertGreaterThan(diff.diff("rspace", "rspace "), 0)
	lt.assertGreaterThan(diff.diff("case-sensitive", "CaSe-SeNsitIvE"), 0)
end

return t
