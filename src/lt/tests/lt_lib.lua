local lt = require "lt"
local t = {}

--------------------------
--  Tests
--------------------------

function t:testEqual()
	self:compareBinOp{
		assertion = lt.assertEqual,
		predicate = function(a, b)
			return a == b
		end,
	}
end

function t:testNotEqual()
	self:compareBinOp{
		assertion = lt.assertNotEqual,
		predicate = function(a, b)
			return a ~= b
		end,
	}
end

function t:testGreaterThan()
	local elems = self:getSequence(5)
	self:compareBinOp{
		assertion = lt.assertGreaterThan,
		predicate = function(a, b)
			return a > b
		end,
		arguments = { elems, elems },
	}
end

function t:testGreaterEqual()
	local elems = self:getSequence(5)
	self:compareBinOp{
		assertion = lt.assertGreaterEqual,
		predicate = function(a, b)
			return a >= b
		end,
		arguments = { elems, elems },
	}
end

function t:testLessThan()
	local elems = self:getSequence(5)
	self:compareBinOp{
		assertion = lt.assertLessThan,
		predicate = function(a, b)
			return a < b
		end,
		arguments = { elems, elems },
	}
end

function t:testLessEqual()
	local elems = self:getSequence(5)
	self:compareBinOp{
		assertion = lt.assertLessEqual,
		predicate = function(a, b)
			return a <= b
		end,
		arguments = { elems, elems },
	}
end

function t:testNotEqual()
	self:compareBinOp{
		assertion = lt.assertNotEqual,
		predicate = function(a, b)
			return a ~= b
		end,
	}
end

function t:testRawEqual()
	self:compareBinOp{
		assertion = lt.assertRawEqual,
		predicate = function(a, b)
			return rawequal(a, b)
		end,
	}
end

function t:testNotRawEqual()
	self:compareBinOp{
		assertion = lt.assertNotRawEqual,
		predicate = function(a, b)
			return not rawequal(a, b)
		end,
	}
end

function t:testNil()
	self:compareOp{
		assertion = lt.assertNil,
		predicate = function(a)
			return a == nil
		end,
	}
end

function t:testNotNil()
	self:compareOp{
		assertion = lt.assertNotNil,
		predicate = function(a)
			return a ~= nil
		end,
	}
end

function t:testTrue()
	self:compareOp{
		assertion = lt.assertTrue,
		predicate = function(a)
			return not not a
		end,
	}
end

function t:testFalse()
	self:compareOp{
		assertion = lt.assertFalse,
		predicate = function(a)
			return not a
		end,
	}
end

function t:testIsOfType()
	self:compareBinOp{
		assertion = lt.assertType,
		predicate = function(a, b)
			return type(a) == b
		end,
		filters = {
			function(a) return a end,
			function(b) return type(b) end,
		},
	}
end

function t:testIsNotOfType()
	self:compareBinOp{
		assertion = lt.assertNotType,
		predicate = function(a, b)
			return type(a) ~= b
		end,
		filters = {
			function(a) return a end,
			function(b) return type(b) end,
		},
	}
end

function t:testIsIn()
	local elems = self:getSequence(5)
	local tables = {}
	for i = 1, 5 do
		tables[i] = self:getSequence(i-1)
	end
	self:compareBinOp{
		assertion = lt.assertValue,
		predicate = function(a, b)
			for k, v in pairs(b) do
				if v == a then
					return true
				end
			end
			return false
		end,
		arguments = {
			elems,
			tables,
		},
	}
	local ok, err = pcall(lt.assertValue, 2, 5)
	assert(not ok and err:find('table'))
end

function t:testIsNotIn()
	local elems = self:getSequence(5)
	local tables = {}
	for i = 1, 5 do
		tables[i] = self:getSequence(i-1)
	end
	self:compareBinOp{
		assertion = lt.assertNotValue,
		predicate = function(a, b)
			for k, v in pairs(b) do
				if v == a then
					return false
				end
			end
			return true
		end,
		arguments = {
			elems,
			tables,
		},
	}
	local ok, err = pcall(lt.assertNotValue, 2, 5)
	assert(not ok and err:find('table'))
end

function t:testRaises()
	local called
	local assertIsOdd = function(number)
		called = true
		if number % 2 ~= 1 then
			error("not odd")
		end
	end

	for i = 1, 10 do
		called = false
		if i % 2 ~= 1 then
			local errobj = lt.assertRaises(assertIsOdd, i)
			assert(errobj:find("not odd"))
		else
			assert(not pcall(lt.assertRaises, assertIsOdd, i))
		end
		assert(called)
	end
	local ok, err = pcall(lt.assertRaises, 23)
	assert(not ok and err:find('function'))
end

function t:testSubstringOrNot()
	local needles = {"a", "b", "c", "x", "wxyz", "ab", "abc", "bc", "bcx", "xbc"}
	local haystacks = {"", "b", "abc"}
	-- assertSubstring
	self:compareBinOp{
		assertion = lt.assertSubstring,
		predicate = function(needle, haystack)
			return string.find(haystack, needle) ~= nil
		end,
		arguments = {
			needles,
			haystacks,
		},
	}
	local testargerror = function(...)
		local ok, err = pcall(lt.assertSubstring, ...)
		assert(not ok and err:find('string expected'))
	end
	testargerror(nil, nil)
	testargerror('x', nil)
	testargerror(nil, 'x')
	-- assertNotSubstring
	self:compareBinOp{
		assertion = lt.assertNotSubstring,
		predicate = function(needle, haystack)
			return string.find(haystack, needle) == nil
		end,
		arguments = {
			needles,
			haystacks,
		},
	}
	testargerror = function(...)
		local ok, err = pcall(lt.assertNotSubstring, ...)
		assert(not ok and err:find('string expected'))
	end
	testargerror(nil, nil)
	testargerror('x', nil)
	testargerror(nil, 'x')
end

function t:testDeepEqual()
	local tables = {
		{},
		{},
		{a = 123},
		{b = 123},
		{a = 123, b = 345},
		{a = 456, b = 546},
		{a = 123, b = 777},
		{a = 123, b = 345},
		{a = 123, b = 345, c = 678},
	}
	-- assertDeepEqual
	local predicate = function(t1, t2)
		for k, v1 in pairs(t1) do
			if v1 ~= t2[k] then
				return false
			end
		end
		for k, v2 in pairs(t2) do
			if v2 ~= t1[k] then
				return false
			end
		end
		return true
	end
	self:compareBinOp{
		assertion = lt.assertDeepEqual,
		predicate = predicate,
		arguments = {
			tables,
			tables,
		},
	}
	local testargerror = function(...)
		local ok, err = pcall(lt.assertDeepEqual, ...)
		assert(not ok and err:find('table expected'))
	end
	testargerror(nil, nil)
	testargerror({}, nil)
	testargerror(nil, {})
	-- assertNotDeepEqual
	local reverse_predicate = function(t1, t2)
		return not predicate(t1, t2)
	end
	self:compareBinOp{
		assertion = lt.assertNotDeepEqual,
		predicate = reverse_predicate,
		arguments = {
			tables,
			tables,
		},
	}
	testargerror = function(...)
		local ok, err = pcall(lt.assertNotDeepEqual, ...)
		assert(not ok and err:find('table expected'))
	end
	testargerror(nil, nil)
	testargerror({}, nil)
	testargerror(nil, {})
end

function t:testUdata()
	local udata = lt.udata()
	assert(type(udata) == 'userdata')
end

-----------------------
-- Auxiliary methods
-----------------------

function t:compareBinOp(t)
	local func = assert(t.assertion)
	local pred = assert(t.predicate)
	local filters = t.filters or {}
	local filter1 = filters[1] or function(a) return a end
	local filter2 = filters[2] or function(b) return b end
	local arguments = t.arguments or {}
	local vs1 = arguments[1] or self:getValues()
	local vs2 = arguments[2] or self:getValues()
	local m, n = #vs1, #vs2
	if arguments[1] == nil then m = m + 1 end
	if arguments[2] == nil then n = n + 1 end
	for i = 1, m do
		local v1 = filter1(vs1[i])
		for j = 1, n do
			local v2 = filter2(vs2[j])
			if pred(v1, v2) then
				func(v1, v2)
			else
				assert(not pcall(func, v1, v2),
					"expected error when calling function with " ..
					tostring(v1) .. " and " .. tostring(v2))
			end
		end
	end
end

function t:compareOp(t)
	local vs1 = self:getValues()
	local func = assert(t.assertion)
	local pred = assert(t.predicate)
	local n = #vs1 + 1 -- v1[n] is nil
	for i = 1, n do
		local v1 = vs1[i]
		if pred(v1) then
			func(v1)
		else
			assert(not pcall(func, v1), "expected error")
		end
	end
end

function t:getValues()
	local tabl = {1, 2, 3}
	local func = function() end
	local coro = coroutine.create(func)
	local user = lt.udata()
	return {
		1234, -- integer
		3.14, -- float
		true, -- boolean
		'ab', -- string
		tabl, -- table
		func, -- function
		coro, -- coroutine
		user, -- userdata
	}
end

function t:getSequence(n)
	local t = {}
	for i = 1, n do
		t[i] = i
	end
	return t
end

return t
