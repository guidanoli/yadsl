package.cpath = package.cpath
                .. ";../../../lib64/?.dll"   -- Windows
                .. ";../../../lib64/?.so"    -- POSIX
                .. ";../../../lib64/?.dylib" -- MacOS

local m = require "luadiff"

assert(m.diff("", "") == 0,
       "Empty strings should have diff equal to zero")
assert(m.diff(" ", " ") == 0,
       "Characters should have diff equal to zero")
assert(m.diff("algo", "algo") == 0,
       "Strings without spaces should have diff equal to zero")
assert(m.diff("algo assim", "algo assim") == 0,
       "Strings with spaces should have diff equal to zero")
assert(m.diff("a", "") ~= 0,
       "Characters and empty strings should have diff different from zero")
assert(m.diff("a", " ") ~= 0,
       "Different characters should have diff different from zero")
assert(m.diff("abc", "abd") ~= 0,
       "Different strings should have diff different from zero")
assert(m.diff("space", "s p a c e") ~= 0,
       "Additional spaces should not make diff different from zero")
assert(m.diff("lspace", " lspace") ~= 0,
       "Left space should not make diff different from zero")
assert(m.diff("rspace", "rspace ") ~= 0,
       "Right space should not make diff different from zero")
assert(m.diff("case-sensitive", "CaSe-SeNsitIvE") ~= 0,
       "Diff should be case-sensitive")

print("Success")