# Create new project

'''
Create new project with name 'prj'
[!] Alters current working directory
'''
def new_project(prj, test = True, python = False, **kwargs):
	import os
	# Check if no additional kwarg was passed
	if len(kwargs) != 0:
		raise Exception("Unknown argument{} {}".format(
		"s" if len(kwargs) > 1 else "",
		', '.join("'{}'".format(k) for k in kwargs.keys())))
	# Check if project name is alphanumeric
	if not prj.isalnum():
		raise Exception("Project name must be alphanumeric")
	# Check if there is a project with same name already
	abspath = os.path.abspath(__file__)
	dname = os.path.dirname(abspath)
	os.chdir(dname)
	os.chdir('src')
	if os.path.exists(prj):
		raise Exception("Project already exists")
	# Create folder named <name>
	os.mkdir(prj)
	os.chdir(prj)
	exts = {
		"source": [('', [''])],
		"test": [('', [''])] if test else [],
		"header": [('', '')],
	}
	def writelist(f, lst):
		f.writelines([line + '\n' for line in lst])
	# Create file <name><fend>.h with __NAME<gend>_H__ guards
	for fend, gend in exts["header"]:
		with open(prj + fend + '.h', 'w') as f:
			guard = "__{}_H__".format(prj.upper() + gend.upper())
			writelist(f, ["#ifndef " + guard, "#define " + guard, "", "#endif"])
	# Create file <name><fend>.c including [<name><hend>.h ...]
	for fend, hends in exts["source"]:
		with open(prj + fend + '.c', 'w') as f:
			writelist(f, ["#include \"{}{}.h\"".format(prj, hend)
			for hend in hends])
	# Create file <name><fend>.test.c including [tester.h <name><hend>.h ...]
	for fend, hends in exts["test"]:
		with open(prj + fend + '.test.c', 'w') as f:
			lines = list()
			headers = [(prj + hend) for hend in hends]
			lines += ["#include \"{}.h\"".format(h) for h in headers]
			lines += ["", "#include \"tester.h\""]
			lines += ["", "const char *TesterHelpStrings[] = {",
			"	\"This is the {} test module\"".format(prj + fend), "};", "",
			"TesterReturnValue TesterInitCallback()",
			"{", "	return TESTER_RETURN_OK;", "}", "",
			"TesterReturnValue TesterParseCallback(const char *command)",
			"{","	return TESTER_RETURN_OK;", "}", "",
			"TesterReturnValue TesterExitCallback()",
			"{","	return TESTER_RETURN_OK;", "}", ""]
			writelist(f, lines)
		with open(prj + fend + '.script', 'w') as f:
			writelist(f, ["# " + prj + fend])
	# Create README.md with <name> title
	with open("README.md", 'w') as f:
		writelist(f, ['# ' + prj])
	# Create CMakeLists.txt with build configurations
	with open("CMakeLists.txt", 'w') as f:
		lines = list()
		sources = [prj + fend + '.c' for fend, _ in exts["source"]]
		headers = [prj + fend + '.h' for fend, _ in exts["header"]]
		lines += ["add_library({} {})".format(prj, " ".join(sources + headers))]
		testnames = [prj + fend for fend, _ in exts["test"]]
		tests = [(t + 'test', t + '.test.c', t + '.script') for t in testnames]
		lines += ["target_include_directories({} PUBLIC ${{CMAKE_CURRENT_SOURCE_DIR}})".format(prj)]
		if python:
			lines += ["set_target_properties({} PROPERTIES".format(prj),
			"\tFOLDER libraries",
			"\tPOSITION_INDEPENDENT_CODE ON)"]
		else:
			lines += ["set_target_properties({} PROPERTIES FOLDER libraries)".format(prj)]
		lines += [""]
		lines += ["if(AA_BUILD_TESTS)"]
		lines += ["\tadd_tester_module({} SOURCES {} LINKS {})".format(tgt, src, prj) for tgt, src, _ in tests]
		lines += ["\tadd_tester_scripts({} SOURCES {})".format(tgt, script) for tgt, _, script in tests]
		lines += ["endif()"]
		if python:
			lines += ["",
			"if(AA_PYTHON_SUPPORT)",
			"\tadd_subdirectory(python)",
			"endif()"]
		writelist(f, lines)
	# Do python subfolder optional setup
	if python:
		pyprj = "py" + prj
		# Create folder named <python>
		os.mkdir("python")
		os.chdir("python")
		# Create CMakeLists.txt
		with open("CMakeLists.txt", 'w') as f:
			writelist(f, ["include(addPythonModule)",
			"add_python_module({} {} {}.py.c)".format(pyprj, pyprj, prj),
			"target_link_libraries({} {} aa)".format(pyprj, prj)])
		# Create file <prj>.py.c including Python.h and <prj>.h
		with open(prj + ".py.c", 'w') as f:
			writelist(f, ["#define PY_SSIZE_T_CLEAN",
			"#include <Python.h>",
			"",
			"#include \"pydefines.h\"",
			"",
			"#include \"{}.h\"".format(prj),
			"",
			"PyModuleDef {}_module = {{".format(pyprj),
			"\tPyModuleDef_HEAD_INIT,",
			"\t\"{}\",".format(pyprj),
			"};",
			"",
			"PyMODINIT_FUNC",
			"PyInit_{}(void)".format(pyprj),
			"{",
			"\tPy_Initialize();",
			"\treturn PyModule_Create(&{}_module);".format(pyprj),
			"}"])
		# Create file <prj>_test.c importing py<prj>
		with open(prj + "_test.py", 'w') as f:
			f.write("from {} import *".format(pyprj))
		os.chdir("..")
	# Add project to root CMakeLists.txt
	os.chdir("..")
	with open("CMakeLists.txt", "a") as f:
		writelist(f, ["add_subdirectory({})".format(prj)])

if __name__ == '__main__':
	import sys
	if len(sys.argv) > 1:
		kwargs = dict()
		for arg in sys.argv[2:]:
			parts = arg.split("=")
			if (len(parts) != 2 or
			len(parts[0]) < 3 or
			len(parts[1]) == 0 or
			parts[0][:2] != "--"):
				print("Invalid parameter {}".format(arg))
				continue
			key = parts[0][2:]
			value = parts[1]
			if key.lower() == "test":
				kwargs["test"] = value.upper() == "YES"
			if key.lower() == "python":
				kwargs["python"] = value.upper() == "YES"
			else:
				print("Invalid key {}".format(key))
		new_project(sys.argv[1], **kwargs)
	else:
		print("Usage: python new.py <prj> [flags]")
		print("--test=[YES/NO], default = YES")
		print("--python=[YES/NO], default = NO")