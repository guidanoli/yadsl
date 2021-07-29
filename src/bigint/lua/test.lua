local lt = require "lt"
local bigint = require "bigint"

local t = { afterAll = bigint.memdb.afterAll }

function t:testRoundtrip()
	for _, integer in pairs{math.mininteger, -1, 0, 1, math.maxinteger} do
		local x = bigint.BigInt(integer)
		lt.assertEqual(x:to_integer(), integer)
		lt.assertEqual(x:to_string(), tostring(integer))
	end
end

function t:testRoundtripMetamorphic()
	local NUM_TESTS = 100
	for TEST_INDEX = 1, NUM_TESTS do
		local integer = math.random(math.mininteger, math.maxinteger)
		local x = bigint.BigInt(integer)
		lt.assertEqual(x:to_integer(), integer)
		lt.assertEqual(x:to_string(), tostring(integer))
	end
end

return t
