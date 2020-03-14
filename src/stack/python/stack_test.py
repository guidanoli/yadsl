from pystack import *

def test_empty():
	st = stack()
	try:
		st.remove()
	except Empty:
		return
	assert False

def test_add_and_remove():
	st = stack()
	assert st.is_empty()
	for i in range(100):
		st.add(i)
		assert not st.is_empty()
	for i in range(99, -1, -1):
		assert not st.is_empty()
		assert i == st.remove()
	assert st.is_empty()