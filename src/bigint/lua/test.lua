local lt = require "lt"
local bigint = require "bigint"
local limits = require "limits"

local t = { afterAll = bigint.memdb.afterAll }

function t:integer_roundtrip(integer)
	local x = bigint.BigInt(integer)
	lt.assertEqual(x:to_integer(), integer)
	lt.assertEqual(tostring(x), tostring(integer))
end

function t:testIntegerRoundtrip()
	for _, integer in pairs{math.mininteger, -1, 0, 1, math.maxinteger} do
		self:integer_roundtrip(integer)
	end
end

function t:testIntegerRoundtripMetamorphic()
	local NUM_TESTS = 100
	for TEST_INDEX = 1, NUM_TESTS do
		local integer = math.random(math.mininteger, math.maxinteger)
		self:integer_roundtrip(integer)
	end
end

function t:testString()
	function teststr(str, estr)
		lt.assertType(str, 'string')
		if estr == nil then
			estr = str
		else
			lt.assertType(estr, 'string')
		end
		local big = bigint.BigInt(str)
		lt.assertEqual(tostring(big), estr)
	end
	for i = -9, 9 do
		local str = tostring(i)
		teststr(str)
		if i >= 0 then
			for _, sign in ipairs{'+', '-'} do
				local esign
				if sign == '+' or i == 0 then
					esign = ''
				else
					esign = sign
				end
				for nzeros = 0, 10 do
					teststr(sign .. string.rep('0', nzeros) .. str, esign .. str)
				end
			end
		end
	end
	do
		teststr(string.rep('1234567890', 10))
	end
end

function t:testIntegerOverflow()
	local str = limits.INTMAX_MAX .. "0"
	local big = bigint.BigInt(str)
	lt.assertEqual("integer overflow", lt.assertRaises(big.to_integer, big))
end

function t:testInvalidString()
	local teststr = function(str)
		lt.assertSubstring("string is ill-formatted",
			lt.assertRaises(bigint.BigInt, str))
	end
	teststr("")
	teststr("+")
	teststr("-")
	teststr("/")
	teststr(":")
	teststr("0:")
	teststr("0/")
	teststr("1:")
	teststr("1/")
	teststr("01:")
	teststr("01/")
end

function t:testStringMetamorphic()
	local NUM_TESTS = 100
	for TEST_INDEX = 1, NUM_TESTS do
		local num = math.random(math.maxinteger)
		local negative = math.random() < 1/3
		local sign
		if negative then
			sign = "-"
		else
			sign = math.random() < 1/3 and "+" or ""
		end
		local nzeros = math.random(0, 3)
		local zeros = string.rep('0', nzeros)
		local numstr = tostring(num)
		local str = sign .. zeros .. numstr
		local x = bigint.BigInt(str)
		lt.assertEqual(x:to_integer(), num * (negative and -1 or 1))
		lt.assertEqual(tostring(x), (negative and "-" or "") .. numstr)
	end
end

return t
