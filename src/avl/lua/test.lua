local lt = require "lt"
local avl = require "avl"

local t = { afterAll = avl.memdb.afterAll }

function t:testEmpty()
	local tree = avl.AVLTree()
end

function t:testFree()
	local tree = avl.AVLTree()
	for i = 1, 10 do tree:insert(i) end
end

function t:testInsertSearchAndRemove()
	local tree = avl.AVLTree()
	lt.assertFalse(tree:remove(123)) --
	lt.assertFalse(tree:search(123)) --
	lt.assertFalse(tree:insert(123)) -- 123
	lt.assertTrue(tree:search(123))  -- 123
	lt.assertTrue(tree:remove(123))  --
	lt.assertFalse(tree:search(123)) --
	lt.assertFalse(tree:insert(123)) -- 123
	lt.assertTrue(tree:search(123))  -- 123
	lt.assertTrue(tree:remove(123))  --
	lt.assertFalse(tree:search(123)) --
end

function t:testInsertSearchAndRemoveMany()
	local tree = avl.AVLTree()
	for i = 1, 10 do
		for j = i, 10 do
			lt.assertFalse(tree:remove(j), j)
			lt.assertFalse(tree:search(j), j)
		end
		lt.assertFalse(tree:insert(i))
		for j = 1, i do
			lt.assertTrue(tree:search(j), j)
		end
	end
	for i = 1, 10 do
		lt.assertTrue(tree:remove(i), i)
	end
end

function t:testReinsert()
	local tree = avl.AVLTree()
	lt.assertFalse(tree:search(123)) -- 
	lt.assertFalse(tree:insert(123)) -- 123
	lt.assertTrue(tree:search(123))  -- 123
	lt.assertTrue(tree:insert(123))  -- 123
	lt.assertTrue(tree:search(123))  -- 123
	lt.assertTrue(tree:remove(123))  --
	lt.assertFalse(tree:search(123)) --
end

return t
