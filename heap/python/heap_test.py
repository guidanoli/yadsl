from pyheap import *

def test_create():
	h = Heap()
	h = Heap(size=127)
	assert h.size() == 127
	def func(a,b):
		return a > b
	h = Heap(f=func)
	h = Heap(f=func, size=127)
	assert h.size() == 127

def test_full():
	h = Heap(size=1)
	h.insert(1)
	try:
		h.insert(2)
	except Full:
		return
	assert False

def test_empty():
	h = Heap()
	try:
		h.extract()
	except Empty:
		return
	assert False

def test_lock():
	from random import random
	l = []
	def evil(a,b):
		l[0].extract()
		return a < b
	h = Heap(f=evil, size=7)
	l.append(h)
	try:
		for i in range(7):
			h.insert(random())
		for i in range(7):
			print(h.extract())
	except Lock:
		return
	assert False

def test_shrink():
	h = Heap(size=15)
	for i in range(7):
		h.insert(i)
	h.resize(7)
	assert h.size() == 7
	try:
		h.resize(3)
	except Shrink:
		assert h.size() == 7
		return
	assert False

def test_ordering():
	from random import random
	def minheap(a,b):
		return a < b
	def maxheap(a,b):
		return a > b
	for func in [minheap, maxheap]:
		h = Heap(f=func, size=100)
		for i in range(100):
			h.insert(random())
		prev = h.extract()
		for i in range(99):
			new = h.extract()
			assert func(prev,new)
			prev = new