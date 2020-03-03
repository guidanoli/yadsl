import perfplot
import numpy

import pyavl

def avl_iter(ds):
	x, s, t = ds
	def f(o):
		pass
	t.iterate(f)
	return True

def set_iter(ds):
	x, s, t = ds
	for si in s:
		pass
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
		avl_iter,
		set_iter,
	],
	n_range=[2**k for k in range(20)],
	labels=[
		"AVL tree",
		"set",
	],
	title='Iterating array performance',
	xlabel='len(x)',
)