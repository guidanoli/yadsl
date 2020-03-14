import perfplot
import numpy
import random

import pyavl

def get_pair(x):
	contains = numpy.random.choice(x)
	contains_not = random.random()
	while contains_not in x:
		contains_not = random.random()
	return contains, contains_not

def avl_query(ds):
	x, s, t = ds
	c, cn = get_pair(x)
	assert t.contains(c)
	assert not t.contains(cn)
	return True

def set_query(ds):
	x, s, t = ds
	c, cn = get_pair(x)
	assert c in s
	assert cn not in s
	return True

def setup_ds(n):
	x = numpy.random.rand(n)
	s = set()
	t = pyavl.AVL()
	for xi in x:
		t.add(xi)
		s.add(xi)
	return (x, s, t)

perfplot.show(
	setup=setup_ds,
	kernels=[
		avl_query,
		set_query,
	],
	n_range=[2**k for k in range(20)],
	labels=[
		"AVL tree",
		"set",
	],
	title='Querying array performance',
	xlabel='len(x)',
)