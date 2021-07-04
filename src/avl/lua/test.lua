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

function t:getTreeSequence(n)
	local tree = avl.AVLTree()
	for i = 1, n do tree:insert(i) end
	return tree
end

function t:testTraverseEmpty()
	local tree = avl.AVLTree()
	local visited = false
	local obj = tree:traverse(function(o)
		visited = true
	end)
	lt.assertFalse(visited)
	lt.assertNil(obj)
end

function t:testTraverseMany()
	local tree = self:getTreeSequence(10)
	local visited = {}
	tree:traverse(function(o)
		table.insert(visited, o)
	end)
	for i = 1, 10 do
		lt.assertEqual(visited[i], i)
	end
end

function t:testTraverseReturnValue()
	local tree = self:getTreeSequence(10)
	local visited = {}
	local obj, nothing = tree:traverse(function(o)
		table.insert(visited, o)
		if o == 6 then return o, 123 end
	end)
	lt.assertNil(nothing)
	lt.assertEqual(obj, 6)
	lt.assertEqual(#visited, 6)
	for i = 1, 6 do lt.assertEqual(visited[i], i) end
end

function t:_testTraverseOrder(order)
	local tree = self:getTreeSequence(10)
	local visited = {}
	tree:traverse(function(o)
		visited[o] = true
	end, order)
	local differs = false
	for i = 1, 10 do
		lt.assertTrue(visited[i])
		if i ~= visited[i] then
			differs = true
		end
	end
	lt.assertTrue(differs)
end

function t:testTraversePreOrder()
	self:_testTraverseOrder("pre")
end

function t:testTraversePostOrder()
	self:_testTraverseOrder("post")
end

return t
