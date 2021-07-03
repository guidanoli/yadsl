local lt = require "lt"
local avl = require "avl"

local t = { afterAll = avl.memdb.afterAll }

function t:testEmpty()
	local tree = avl.AVLTree()
end

return t
