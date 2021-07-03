local argvp = require "argvp"
local lt = require "lt"

local t = { afterAll = argvp.memdb.afterAll }

function t:iterKeywordValues(p, kw)
	local i = 0
	return function()
		i = i + 1
		local val = p:getKeywordArgumentValue(kw, i)
		if val ~= nil then return i, val end
	end
end

function t:getKeywordValues(p, kw)
	local kwvalues = {}
	for i, v in self:iterKeywordValues(p, kw) do
		kwvalues[i] = v
	end
	return kwvalues
end

function t:assertKeywordValues(p, kw, expected)
	local obtained = self:getKeywordValues(p, kw)
	for k, v in pairs(obtained) do
		lt.assertEqual(expected[k], v, k)
	end
	for k, v in pairs(expected) do
		lt.assertEqual(obtained[k], v, k)
	end
end

function t:iterPositionalArguments(p)
	local i = 0
	return function()
		i = i + 1
		local arg = p:getPositionalArgument(i)
		if arg ~= nil then return i, arg end
	end
end

function t:getPositionalArguments(p)
	local posargs = {}
	for i, v in self:iterPositionalArguments(p) do
		posargs[i] = v
	end
	return posargs;
end

function t:assertPositionalArguments(p, expected)
	local obtained = self:getPositionalArguments(p)
	for k, v in pairs(obtained) do
		lt.assertEqual(expected[k], v, k)
	end
	for k, v in pairs(expected) do
		lt.assertEqual(obtained[k], v, k)
	end
end

function t:assertArgs(p, pos, kw)
	self:assertPositionalArguments(p, pos)
	for k, v in pairs(kw) do
		self:assertKeywordValues(p, k, v)
	end
end

function t:testEmpty()
	local p = argvp.ArgumentParser{}
	self:assertArgs(p, {}, {})
	lt.assertFalse(p:hasKeywordArgument('--help'))
end

function t:testOne()
	local p = argvp.ArgumentParser{'--help'}
	self:assertArgs(p, {'--help'}, {})
	lt.assertFalse(p:hasKeywordArgument('--help'))
	p:addKeywordArgument('--help', 0)
	self:assertArgs(p, {}, {['--help'] = {}})
	lt.assertTrue(p:hasKeywordArgument('--help'))
end

function t:testMultiple()
	local p = argvp.ArgumentParser{'sth', '-x', '10', '-y', '20', '40', 'else'}
	lt.assertFalse(p:hasKeywordArgument('-x'))
	lt.assertFalse(p:hasKeywordArgument('-y'))
	self:assertArgs(p, {'sth', '-x', '10', '-y', '20', '40', 'else'}, {})
	p:addKeywordArguments{ ['-x'] = 1, ['-y'] = 2 }
	lt.assertTrue(p:hasKeywordArgument('-x'))
	lt.assertTrue(p:hasKeywordArgument('-y'))
	self:assertArgs(p, {'sth', 'else'}, {['-x'] = {'10'}, ['-y'] = {'20', '40'}})
end

function t:generateArgs(max)
	local args = {}
	local n = math.random(1, max)
	for i = 1, n do
		args[i] = tostring(math.random())
	end
	return args, n
end

function t:testOnlyPositionalMetamorphic()
	math.randomseed(os.time())
	local NUM_TESTS = 100
	for TEST_INDEX = 1, NUM_TESTS do
		local args = self:generateArgs(100)
		local p = argvp.ArgumentParser(args)
		self:assertArgs(p, args, {})
	end
end

function t:testKwargsMetamorphic()
	math.randomseed(os.time())
	local NUM_TESTS = 100
	for TEST_INDEX = 1, NUM_TESTS do
		local posargs = self:generateArgs(50)
		local kwargs = {}
		for i = 1, math.random(1, 50) do
			kwargs['--kw' .. i] = self:generateArgs(5)
		end
		local args = {}
		local posi, posarg = 1, posargs[1]
		local kwarg, kwvals = next(kwargs)
		while true do
			if posarg ~= nil and kwarg == nil or math.random() < 0.5 then
				table.insert(args, posarg)
				posi = posi + 1
				posarg = posargs[posi]
			elseif kwarg ~= nil then
				table.insert(args, kwarg)
				for _, kwval in ipairs(kwvals) do
					table.insert(args, kwval)
				end
				kwarg, kwvals = next(kwargs, kwarg)
			else
				break
			end
		end
		local p = argvp.ArgumentParser(args)
		for kwarg, kwvals in pairs(kwargs) do
			p:addKeywordArgument(kwarg, #kwvals)
		end
		self:assertArgs(p, posargs, kwargs)
	end
end

return t
