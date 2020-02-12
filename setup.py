from pathlib import Path

dirs = []
exts = ["*.pyd", "*.so", "*.dylib"]
for ext in exts:
	for pyd in Path('.').rglob(ext):
		if pyd.parent not in dirs:
			dirs.append(pyd.parent)

with open(".paths", "w") as f:
	f.writelines("\n".join([str(dir.resolve()) for dir in dirs]))