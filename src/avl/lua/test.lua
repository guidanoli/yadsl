local lt = require "lt"
local avl = require "avl"

local t = { afterAll = avl.memdb.afterAll }

function t:testEmpty()
	local tree = avl.AVLTree()
end

function t:testInsert()
	local tree = avl.AVLTree()
	lt.assertFalse(tree:insert(123))
end

function t:testReinsert()
	local tree = avl.AVLTree()
	lt.assertFalse(tree:insert(123))
	lt.assertTrue(tree:insert(123))
end

return t
