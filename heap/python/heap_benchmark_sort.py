import perfplot
import numpy

import pyheap

def heap_sort(arg):
	x, y = arg
	h = pyheap.Heap(size = len(x))
	for xi in x:
		h.insert(xi)
	i = 0
	while True:
		try:
			y[i] = h.extract()
		except pyheap.Empty:
			break
		i += 1
	return y

def list_sort(arg):
	x, y = arg
	y = x[:]
	y.sort()
	return y
	
def numpy_sort(arg):
	x, y = arg
	return numpy.sort(x)

def heap_and_array(n):
	x = numpy.random.rand(n)
	y = numpy.zeros(n)
	return x, y
	
perfplot.show(
	setup=heap_and_array,
	kernels=[
		heap_sort,
		numpy_sort,
		list_sort
	],
	labels=[
		"heap",
		"numpy",
		"list"
	],
	title='Sorting array performance',
	n_range=[2**k for k in range(20)],
	xlabel='len(x)',
)