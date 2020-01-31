# Create new project

'''
Create new project with name 'prj'
[!] Alters current working directory
'''
def new_project(prj, no_test = False, **kwargs):
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
	if os.path.exists(prj):
		raise Exception("Project already exists")
	# Create folder named <name>
	os.mkdir(prj)
	os.chdir(prj)
	exts = {
		"source": [('', [''])],
		"test": [] if no_test else [('', [''])],
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
			headers = ["tester"] + [(prj + hend) for hend in hends]
			lines += ["#include \"{}.h\"".format(h) for h in headers]
			lines += ["", "const char *TesterHelpStrings[] = NULL;", "",
			"TesterReturnValue TesterInitCallback()",
			"{", "    return TESTER_RETURN_OK;", "}", "",
			"TesterReturnValue TesterParseCallback(const char *command)",
			"{","    return TESTER_RETURN_OK;", "}", "",
			"TesterReturnValue TesterExitCallback()",
			"{","    return TESTER_RETURN_OK;", "}", ""]
			writelist(f, lines)
		with open(prj + fend + '.script', 'w') as f:
			pass # Just create the file
	# Create README.md with <name> title
	with open("README.md", 'w') as f:
		writelist(f, ['# ' + prj])
	# Create CMakeLists.txt with build configurations
	with open("CMakeLists.txt", 'w') as f:
		lines = list()
		sources = ["{}{}.c".format(prj, s[0]) for s in exts["source"]]
		headers = ["{}{}.h".format(prj, h[0]) for h in exts["header"]]
		lines += ["add_library({} {})".format(prj, " ".join(sources + headers))]
		tests = [(prj + t[0] + 'test', prj + t[0] + '.test.c', t[0]) for t in exts["test"]]
		lines += ["add_executable({} {})".format(t, s) for t, s, _ in tests]
		lines += ["target_link_libraries({} tester {})".format(t, prj) for t, _, _ in tests]
		lines += ["get_filename_component({}_SCRIPT_PATH {}.script ABSOLUTE)"
		.format(target.upper(), target) for target, _, _ in tests]
		lines += ["add_test(NAME {}Test COMMAND {} ${{{}_SCRIPT_PATH}})"
		.format(prj.title() + ext.title(), t, t.upper()) for t, _, ext in tests]
		lines += ["target_include_directories({} PUBLIC ${{CMAKE_CURRENT_SOURCE_DIR}})".format(prj)]
		writelist(f, lines)
	# Add project to root CMakeLists.txt
	os.chdir("..")
	with open("CMakeLists.txt", "a") as f:
		writelist(f, ["add_subdirectory({})".format(prj)])

if __name__ == '__main__':
	import sys
	if len(sys.argv) > 1:
		kwargs = dict()
		kwargs["no_test"] = "--no-test" in sys.argv[1:]
		new_project(sys.argv[1], **kwargs)
	else:
		print("Usage: \tpython new.py <prj> [flags]")
		print("--no-test \tDoes not add test to project automatically")