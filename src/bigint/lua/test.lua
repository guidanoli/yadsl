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

	local testaddx = function(...)
		for _, i in ipairs{1, 20, 200} do
			testadd(i, ...)
		end
	end

	-- a = 0 && b = 0
	testaddx(0, 0, 0)

	-- a /= 0 && b = 0
	testaddx(1, 0, 1)

	-- a = 0 && b /= 0
	testaddx(0, 1, 1)

	-- a > 0 && b > 0
	testaddx(2, 1, 3) -- |a| > |b|
	testaddx(1, 2, 3) -- |a| < |b|

	-- a < 0 && b < 0
	testaddx(2, 1, 3, '-', '-', '-') -- |a| > |b|
	testaddx(1, 2, 3, '-', '-', '-') -- |a| < |b|

	-- a > 0 && b < 0
	testaddx(2, 1, 1, '+', '-', '+') -- |a| > |b|
	testaddx(1, 2, 1, '+', '-', '-') -- |a| < |b|

	-- a < 0 && b > 0
	testaddx(2, 1, 1, '-', '+', '-') -- |a| > |b|
	testaddx(1, 2, 1, '-', '+', '+') -- |a| < |b|

	do
		-- a >> b and a << b
		local a = self:makeBig(2000, 1)
		local b = bigint.BigInt(1)
		local c = bigint.BigInt(string.rep(1, 1999) .. 2)
		lt.assertEqual(a + b, c)
		lt.assertEqual(b + a, c)
	end
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

	local testsubtractx = function(...)
		for _, i in ipairs{1, 20, 200} do
			testsubtract(i, ...)
		end
	end

	-- a = 0 && b = 0
	testsubtractx(0, 0, 0)

	-- a /= 0 && b = 0
	testsubtractx(1, 0, 1)

	-- a = 0 && b /= 0
	testsubtractx(0, 1, 1, '+', '+', '-')

	-- a > 0 && b > 0
	testsubtractx(3, 1, 2) -- |a| > |b|
	testsubtractx(1, 3, 2, '+', '+', '-') -- |a| < |b|

	-- a < 0 && b < 0
	testsubtractx(3, 1, 2, '-', '-', '-') -- |a| > |b|
	testsubtractx(1, 3, 2, '-', '-', '+') -- |a| < |b|

	-- a > 0 && b < 0
	testsubtractx(2, 1, 3, '+', '-', '+') -- |a| > |b|
	testsubtractx(1, 2, 3, '+', '-', '+') -- |a| < |b|

	-- a < 0 && b > 0
	testsubtractx(2, 1, 3, '-', '+', '-') -- |a| > |b|
	testsubtractx(1, 2, 3, '-', '+', '-') -- |a| < |b|

	-- a = b
	testsubtractx(7, 7, 0)

	do
		-- a >> b and a << b
		local a = self:makeBig(2000, 1) -- 1...111
		local b = bigint.BigInt(-1) -- -1
		local c = bigint.BigInt(string.rep(1, 1999) .. 2) -- 1...112
		lt.assertEqual(a - b, c) -- 1...111 - (-1) = 1...112
		lt.assertEqual(b - a, -c) -- -1 - 1...111 = -1...112
	end
	
	do
		-- a ~ b
		local a = self:makeBig(2000, 1) -- 1...111
		local b = bigint.BigInt(string.rep(1, 1999) .. 2) -- 1...112
		local c = bigint.BigInt(1) -- 1
		lt.assertEqual(a - b, -c) -- 1...111 - 1...112 = -1
		lt.assertEqual(b - a, c) -- 1...112 - 1...111 = 1
	end
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

	-- n >> 1
	testopposite(200, 1, true)
	testopposite(200, 1, false)
end

return t
