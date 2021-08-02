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

-- Make BigInt from a repeated n times
-- 'sign' can be either '' (positive by default), '+' (positive) or '-' (negative)
function t:makeBig(n, a, sign)
	sign = sign or '+'
	return bigint.BigInt(sign .. string.rep(a, n))
end

function t:testAdd()
	-- Tests a + b = c
	-- a, b, and c are strings repeated n times
	-- asign, bsign and csign indicate the sign of these numbers
	local testadd = function(n, a, b, c, asign, bsign, csign)
		lt.assertEqual(self:makeBig(n, a, asign) +
		               self:makeBig(n, b, bsign),
					   self:makeBig(n, c, csign))
	end

	-- na = 0 && nb = 0
	testadd(1, 0, 0, 0)

	-- na /= 0 && nb = 0
	testadd(5, 1, 0, 1) -- one digit
	testadd(20, 1, 0, 1)

	-- na = 0 && nb /= 0
	testadd(5, 0, 1, 1) -- one digit
	testadd(20, 0, 1, 1)

	-- na > 0 && nb > 0
	testadd(5, 2, 1, 3) -- |a| > |b|, one digit
	testadd(5, 1, 2, 3) -- |a| < |b|, one digit
	testadd(20, 2, 1, 3) -- |a| > |b|
	testadd(20, 1, 2, 3) -- |a| < |b|

	-- na < 0 && nb < 0
	testadd(5, 2, 1, 3, '-', '-', '-') -- |a| > |b|, one digit
	testadd(5, 1, 2, 3, '-', '-', '-') -- |a| < |b|, one digit
	testadd(20, 2, 1, 3, '-', '-', '-') -- |a| > |b|
	testadd(20, 1, 2, 3, '-', '-', '-') -- |a| < |b|

	-- na > 0 && nb < 0
	testadd(5, 2, 1, 1, '+', '-', '+') -- |a| > |b|, one digit
	testadd(5, 1, 2, 1, '+', '-', '-') -- |a| < |b|, one digit
	testadd(20, 2, 1, 1, '+', '-', '+') -- |a| > |b|
	testadd(20, 1, 2, 1, '+', '-', '-') -- |a| < |b|

	-- na < 0 && nb > 0
	testadd(5, 2, 1, 1, '-', '+', '-') -- |a| > |b|, one digit
	testadd(5, 1, 2, 1, '-', '+', '+') -- |a| < |b|, one digit
	testadd(20, 2, 1, 1, '-', '+', '-') -- |a| > |b|
	testadd(20, 1, 2, 1, '-', '+', '+') -- |a| < |b|
end

function t:testSubtract()
	-- Tests a - b = c
	-- a, b, and c are strings repeated n times
	-- asign, bsign and csign indicate the sign of these numbers
	local testsubtract = function(n, a, b, c, asign, bsign, csign)
		lt.assertEqual(self:makeBig(n, a, asign) -
		               self:makeBig(n, b, bsign),
					   self:makeBig(n, c, csign))
	end

	-- na = 0 && nb = 0
	testsubtract(1, 0, 0, 0)

	-- na /= 0 && nb = 0
	testsubtract(5, 1, 0, 1) -- one digit
	testsubtract(20, 1, 0, 1)

	-- na = 0 && nb /= 0
	testsubtract(5, 0, 1, 1, '+', '+', '-') -- one digit
	testsubtract(20, 0, 1, 1, '+', '+', '-')

	-- na > 0 && nb > 0
	testsubtract(5, 3, 1, 2) -- |a| > |b|, one digit
	testsubtract(5, 1, 3, 2, '+', '+', '-') -- |a| < |b|, one digit
	testsubtract(20, 3, 1, 2) -- |a| > |b|
	testsubtract(20, 1, 3, 2, '+', '+', '-') -- |a| < |b|

	-- na < 0 && nb < 0
	testsubtract(5, 3, 1, 2, '-', '-', '-') -- |a| > |b|, one digit
	testsubtract(5, 1, 3, 2, '-', '-', '+') -- |a| < |b|, one digit
	testsubtract(20, 3, 1, 2, '-', '-', '-') -- |a| > |b|
	testsubtract(20, 1, 3, 2, '-', '-', '+') -- |a| < |b|

	-- na > 0 && nb < 0
	testsubtract(5, 2, 1, 3, '+', '-', '+') -- |a| > |b|, one digit
	testsubtract(5, 1, 2, 3, '+', '-', '+') -- |a| < |b|, one digit
	testsubtract(20, 2, 1, 3, '+', '-', '+') -- |a| > |b|
	testsubtract(20, 1, 2, 3, '+', '-', '+') -- |a| < |b|

	-- na < 0 && nb > 0
	testsubtract(5, 2, 1, 3, '-', '+', '-') -- |a| > |b|, one digit
	testsubtract(5, 1, 2, 3, '-', '+', '-') -- |a| < |b|, one digit
	testsubtract(20, 2, 1, 3, '-', '+', '-') -- |a| > |b|
	testsubtract(20, 1, 2, 3, '-', '+', '-') -- |a| < |b|
end

function t:testOpposite()
	-- Test -a = a
	local testopposite = function(n, a, positive)
		local asign = positive and '+' or '-'
		local bsign = positive and '-' or '+'
		lt.assertEqual(-self:makeBig(n, a, asign),
		               self:makeBig(n, a, bsign))
	end
	
	-- n = 0
	testopposite(1, 0, true)

	-- n = 1
	testopposite(1, 1, true)
	testopposite(1, 1, false)

	-- n > 1
	testopposite(20, 1, true)
	testopposite(20, 1, false)
end

return t
