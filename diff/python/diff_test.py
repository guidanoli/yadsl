from pydiff import diff

def test_empty():
	assert diff("","") == 0

def test_equal():
	assert diff(" "," ") == 0
	assert diff("a","a") == 0
	assert diff("abc123","abc123") == 0
	assert diff("good ol' string","good ol' string") == 0

def test_different():
	assert diff("a", "") != 0
	assert diff("a", " ") != 0
	assert diff("", " ") != 0
	assert diff("abc", "abd") != 0
	assert diff("space", "s p a c e") != 0
	assert diff("lspace", " lspace") != 0
	assert diff("rspace", "rspace ") != 0
	assert diff("case", "CaSe") != 0