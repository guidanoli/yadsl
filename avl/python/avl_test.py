from pyavl import *
from random import random

def test_empty():
	t = AVL()
	# should not contain anything
	assert not t.contains(0)
	def f(*args, **kwargs):
		return 123
	# should not find anything
	assert t.iterate(f) is None

def test_adding():
	t = AVL()
	# create random input
	x = [random() for i in range(1000)]
	# remove duplicates
	x = list(dict.fromkeys(x))
	# should not contain any
	for xi in x:
		assert not t.contains(xi)
	# should not FIND to remove
	for xi in x:
		assert not t.remove(xi)
	# should add all
	for xi in x:
		assert t.add(xi)
	# should contain all
	for xi in x:
		assert t.contains(xi)
	# should not add duplicates
	for xi in x:
		assert not t.add(xi)
	# should FIND and remove all
	for xi in x:
		assert t.remove(xi)
	# should not contain any
	for xi in x:
		assert not t.contains(xi)
	# should not FIND to remove
	for xi in x:
		assert not t.remove(xi)
	# should add all again
	for xi in x:
		assert not t.add(xi)
	# let destroy with x

def test_iterate_crescent():
	l = []
	# default cmp is o1 - o2
	t = AVL()
	def add_to_list(o):
		l.append(o)
	# create random input
	x = [random() for i in range(1000)]
	# remove duplicates
	x = list(dict.fromkeys(x))
	# should add all
	for xi in x:
		assert t.add(xi)
	# add to list in crescent order
	t.iterate(add_to_list)
	# should have gone through all numbers
	assert len(l) == len(x)
	# should be in crescent order
	for i in range(len(l)-1):
		assert l[i] <= l[i+1]
		
def test_iterate_decrescent():
	l = []
	# custom cmp
	def cmp(o1, o2):
		return o2 - o1
	# AVL with custom cmp function
	t = AVL(cmp)
	def add_to_list(o):
		l.append(o)
	# create random input
	x = [random() for i in range(1000)]
	# remove duplicates
	x = list(dict.fromkeys(x))
	# should add all
	for xi in x:
		assert t.add(xi)
	# add to list in decrescent order
	t.iterate(add_to_list)
	# should have gone through all numbers
	assert len(l) == len(x)
	# should be in decrescent order
	for i in range(len(l)-1):
		assert l[i] >= l[i+1]