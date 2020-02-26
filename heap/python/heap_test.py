from pyheap import *

def test_create():
	h = Heap()
	h = Heap(size=127)
	def func(a,b):
		return a > b
	h = Heap(f=func)
	h = Heap(f=func, size=127)