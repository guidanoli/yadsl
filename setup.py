from pathlib import Path
from os import chdir

# Collect all .pyd paths
pyd_dirs = []
for pyd in Path('.').rglob('*.pyd'):
	if pyd.parent not in pyd_dirs:
		pyd_dirs.append(pyd.parent)

# Write .paths file
with open(".paths", "w") as f:
	f.writelines([str(dir.resolve()) for dir in pyd_dirs])