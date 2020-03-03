import perfplot
import numpy

import pyavl

def avl_delete(ds):
	x, s, t = ds
	rx = numpy.random.choice(x)
	t.remove(rx)
	t.add(rx)
	return True

def set_delete(ds):
	x, s, t = ds
	rx = numpy.random.choice(x)
	s.remove(rx)
	s.add(rx)
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
		avl_delete,
		set_delete,
	],
	n_range=[2**k for k in range(20)],
	labels=[
		"AVL tree",
		"set",
	],
	title='Deleting item performance',
	xlabel='len(x)',
)