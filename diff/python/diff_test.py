import pydiff

def test_empty():
	assert pydiff.diff("","") == 0

def test_equal():
	assert pydiff.diff(" "," ") == 0
	assert pydiff.diff("a","a") == 0
	assert pydiff.diff("abc123","abc123") == 0
	assert pydiff.diff("good ol' string","good ol' string") == 0

def test_different():
	assert pydiff.diff("a", "") != 0
	assert pydiff.diff("a", " ") != 0
	assert pydiff.diff("", " ") != 0
	assert pydiff.diff("abc", "abd") != 0
	assert pydiff.diff("space", "s p a c e") != 0
	assert pydiff.diff("lspace", " lspace") != 0
	assert pydiff.diff("rspace", "rspace ") != 0
	assert pydiff.diff("case", "CaSe") != 0