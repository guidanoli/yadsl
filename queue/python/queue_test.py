import pyqueue

def test_empty():
	q = pyqueue.Queue()
	assert q.is_empty()

def test_queue_one():
	q = pyqueue.Queue()
	q.queue(1)
	assert not q.is_empty()
	assert q.dequeue() == 1
	assert q.is_empty()

def test_queue_none():
	q = pyqueue.Queue()
	q.queue(None)
	assert not q.is_empty()
	assert q.dequeue() is None
	assert q.is_empty()

def test_queue_more():
	q = pyqueue.Queue()
	q.queue(1)
	q.queue('true')
	q.queue(True)
	assert q.dequeue() == 1
	assert q.dequeue() == 'true'
	assert q.dequeue() == True
	assert q.is_empty()