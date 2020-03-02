import perfplot
import numpy

import pyavl

def avl_iter(d):
	def f(o):
		return None
	t = d['tree']
	t.iterate(f)
	return True

def list_iter(d):
	for li in d["list"]:
		pass
	return True

def numpy_iter(d):
	for ai in d["array"]:
		pass
	return True
	
def get_data(n):
	x = numpy.random.rand(n)
	t = pyavl.AVL()
	for xi in x:
		t.add(xi)
	l = list(x)
	d = dict()
	d["array"] = x
	d["tree"] = t
	d["list"] = l
	return d
	
perfplot.show(
	setup=get_data,
	kernels=[
		avl_iter,
		numpy_iter,
		list_iter,
	],
	labels=[
		"AVL tree",
		"numpy",
		"list"
	],
	title='Iterating array performance',
	n_range=[2**k for k in range(20)],
	xlabel='len(x)',
)