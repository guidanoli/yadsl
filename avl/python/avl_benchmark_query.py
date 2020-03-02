import perfplot
import numpy

import pyavl

def avl_query(arg):
	x, xi, t = arg
	return t.contains(xi)

def list_query(arg):
	x, xi, t = arg
	return xi in x
	
def numpy_query(arg):
	x, xi, t = arg
	return numpy.isin(xi, x)
	
def array_and_tree(n):
	x = numpy.random.rand(n)
	t = pyavl.AVL()
	for xi in x:
		t.add(xi)
	xi = numpy.random.choice(x)
	return (x, xi, t)
	
perfplot.show(
	setup=array_and_tree,
	kernels=[
		avl_query,
		numpy_query,
		list_query
	],
	labels=[
		"AVL tree",
		"numpy",
		"list"
	],
	title='Querying array performance',
	n_range=[2**k for k in range(20)],
	xlabel='len(x)',
)