local lt = require "lt"
local t = { n = 0, m = 0, numbers = {}, min = 1, max = 10 }

function t:beforeAll()
	lt.assertEqual(self.n, 0)
end

function t:beforeEach()
	lt.assertEqual(self.m, #self.numbers)
	self.n = self.n + 1
end

for i = t.min, t.max do
	t['test' .. i] = function(self)
		self.numbers[#self.numbers + 1] = i
		lt.assertEqual(self.n, #self.numbers)
	end
end

function t:afterEach()
	self.m = self.n
end

function t:afterAll()
	table.sort(self.numbers)
	lt.assertEqual(#self.numbers, self.max)
	for i, v in pairs(self.numbers) do
		lt.assertEqual(i, v)
	end
end

return t
