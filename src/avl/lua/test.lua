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

function t:newRandomTable(maxn)
	-- The probability that this table
	-- does not have repeated values is
	-- 1 - 1/n!, which for n = 5 is
	-- is already very high (99.1%).
	local t = {}
	local n = math.random(maxn)
	for i = 1, n do
		t[i] = math.random(maxn)
	end
	return t
end

function t:testMetamorphic()
	local t = self:newRandomTable(100)
	local added = {}
	local added_inv = {}
	local tree = avl.AVLTree()
	for i, v in ipairs(t) do
		local exists = tree:insert(v)
		if exists then
			lt.assertIsIn(v, added)
		else
			lt.assertIsNotIn(v, added)
			table.insert(added, v)
			added_inv[v] = true
		end
	end
	local visited = {}
	tree:traverse(function(o)
		table.insert(visited, o)
	end)
	table.sort(added)
	lt.assertEqual(#added, #visited)
	for i = 1, #added do
		lt.assertEqual(added[i], visited[i], i)
	end
	for i = 1, 100 do
		local remove = math.random() < 0.5
		if added_inv[i] then
			lt.assertTrue(tree:search(i))
			if remove then
				lt.assertTrue(tree:remove(i))
			end
		else
			lt.assertFalse(tree:search(i))
			if remove then
				lt.assertFalse(tree:remove(i))
			end
		end
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

function t:testComparisonError()
	local tree = self:getTreeSequence(10)
	local msg = 'uncomparable'
	local compare_cb = function(o)
		error(msg)
	end
	local ucomp = {}
	setmetatable(ucomp, {
		__eq = compare_cb,
		__le = compare_cb,
		__lt = compare_cb,
	})
	lt.assertRaisesRegex(msg, tree.insert, tree, ucomp)
	lt.assertRaisesRegex(msg, tree.search, tree, ucomp)
	lt.assertRaisesRegex(msg, tree.remove, tree, ucomp)
end

function t:testComparisonErrorNil()
	local tree = self:getTreeSequence(10)
	local msg = 'comparison failed'
	local compare_cb = function(o) error() end
	local ucomp = {}
	setmetatable(ucomp, {
		__eq = compare_cb,
		__le = compare_cb,
		__lt = compare_cb,
	})
	lt.assertRaisesRegex(msg, tree.insert, tree, ucomp)
	lt.assertRaisesRegex(msg, tree.search, tree, ucomp)
	lt.assertRaisesRegex(msg, tree.remove, tree, ucomp)
end

function t:testTraversalError()
	local tree = self:getTreeSequence(10)
	local visited
	local msg = 'untraversable'
	local traverse_cb = function(o)
		table.insert(visited, o)
		error(msg)
	end
	for _, order in ipairs{'pre', 'in', 'post'} do
		visited = {}
		lt.assertRaisesRegex(msg, tree.traverse, tree, traverse_cb, order)
		lt.assertEqual(#visited, 1)
	end
end

function t:testTraversalErrorNil()
	local tree = self:getTreeSequence(10)
	local visited
	local msg = 'traversing callback failed'
	local traverse_cb = function(o)
		table.insert(visited, o)
		error()
	end
	for _, order in ipairs{'pre', 'in', 'post'} do
		visited = {}
		lt.assertRaisesRegex(msg, tree.traverse, tree, traverse_cb, order)
		lt.assertEqual(#visited, 1)
	end
end


function t:testLock()
	local tree = self:getTreeSequence(10)
	local compared = 0
	local compare_cb = function(a, b)
		compared = compared + 1
		tree:remove(a)
		tree:remove(b)
	end
	local trigger = {}
	setmetatable(trigger, {
		__eq = compare_cb,
		__le = compare_cb,
		__lt = compare_cb,
	})
	local msg = 'locked'
	lt.assertEqual(compared, 0)
	lt.assertRaisesRegex(msg, tree.insert, tree, trigger)
	lt.assertEqual(compared, 1)
	lt.assertRaisesRegex(msg, tree.search, tree, trigger)
	lt.assertEqual(compared, 2)
	lt.assertRaisesRegex(msg, tree.remove, tree, trigger)
	lt.assertEqual(compared, 3)
	lt.assertRaisesRegex(msg, tree.traverse, tree, compare_cb)
	lt.assertEqual(compared, 4)
end

return t
