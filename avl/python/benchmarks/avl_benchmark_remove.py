import perfplot
import numpy
import random

import pyavl

def avl_insert(ds):
	x, s, t = ds
	r = random.random()
	added = t.add(r)
	return True

def set_insert(ds):
	x, s, t = ds
	r = random.random()
	s.add(r)
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
		avl_insert,
		set_insert,
	],
	n_range=[2**k for k in range(20)],
	labels=[
		"AVL tree",
		"set",
	],
	title='Inserting in array performance',
	xlabel='len(x)',
)