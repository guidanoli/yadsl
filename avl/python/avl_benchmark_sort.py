import perfplot
import numpy

import pyavl

def avl_sort(x):
	t = pyavl.AVL()
	for xi in x:
		t.add(xi)
	l = []
	def f(num):
		l.append(num)
	t.iterate(f)
	return l

def list_sort(x):
	l = list(x)
	l.sort()
	return l
	
def numpy_sort(x):
	return numpy.sort(x)
	
perfplot.show(
	setup=numpy.random.rand,
	kernels=[
		avl_sort,
		numpy_sort,
		list_sort
	],
	n_range=[2**k for k in range(20)],
	labels=[
		"AVL tree",
		"numpy",
		"list"
	],
	title='Sorting array performance',
	xlabel='len(x)',
)