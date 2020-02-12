import sys
with open(".paths", "r") as f:
	for line in f:
		sys.path.append(line.replace('\n', ''))
###############################################################################
print(sys.path)

import pydiff

def test_empty():
	assert pydiff.diff("","") == 0