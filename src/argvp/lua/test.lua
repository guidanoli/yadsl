local argvp = require "argvp"
local lt = require "lt"

local t = { afterAll = argvp.memdb.afterAll }

function t:testEmpty()
	local p = argvp.ArgumentParser{}
end

function t:testOne()
	local p = argvp.ArgumentParser{'--help'}
	p:addKeywordArgument('--help', 0)
end

function t:testMultiple()
	local p = argvp.ArgumentParser{'sth', '-x', '10', '-y', '20', '40', 'else'}
	p:addKeywordArguments{ ['-x'] = 1, ['-y'] = 2 }
	lt.assertEqual(p:getPositionalArgumentCount(), 2);
	lt.assertEqual(p:getPositionalArgument(1), 'sth');
	lt.assertEqual(p:getPositionalArgument(2), 'else');
end

return t
