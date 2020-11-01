#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

/**
 * @brief Memory Failing Simulator
 * This program intends to simulate memory allocation failure by
 * passing special arguments to the tester module that configure
 * the Memory Debugger in such a way that certain calls to malloc
 * and realloc will fail (malloc_index.e. return NULL)
 * Usage:
 * 
 * memfail <tester-module-file-path> <tester-script>
 * 
 * Returns the number of tests that have failed.
*/

/**
 * @brief Concatenates two strings dynamically
 * @param a first string
 * @param is_a_dyn if a can be deallocated after concatenation
 * @param b second string
 * @param is_b_dyn if b can be deallocated after concatenation
 * @return newly concatenated string or NULL
*/
char* strcatdyn(char* a, bool is_a_dyn, char* b, bool is_b_dyn)
{
	char* s = malloc(strlen(a) + strlen(b) + 1);
	if (s == NULL) {
		fprintf(stderr, "Bad malloc in strcatdyn\n");
		exit(1);
	}
	strcpy(s, a);
	strcat(s, b);
	if (is_a_dyn)
		free(a);
	if (is_b_dyn)
		free(b);
	return s;
}

/**
 * @brief Counts the number of '\n' in a file
 * @param filename file name
 * @return number of '\n' in the file
*/
size_t count_lines(const char* filename)
{
	FILE* f = fopen(filename, "r");
	size_t linecnt = 0;
	if (f == NULL) {
		fprintf(stderr, "Could not open %s\n", filename);
		exit(1);
	} else {
		while (true) {
			char c = fgetc(f);
			if (c == EOF)
				break;
			else if (c == '\n')
				++linecnt;
		}
		fclose(f);
	}
	return linecnt;
}

/**
 * @brief Modify command in order to redirect all output to the corresponding
 * null of the system
 * @param command command to be modified
 * @return new command
*/
char* redirect_stdout_to_null_device(char* command)
{
#if defined(_WIN32) || defined(WIN32)
	command = strcatdyn(command, true, " >nul", false);
#else
	command = strcatdyn(command, true, " >/dev/null", false);
#endif
	return command;
}

int main(int argc, char** argv) {
	char tmpfilename[L_tmpnam];
	char indexstrbuf[20];
	char* command;
	int ret, fail_count = 0;
	size_t malloc_count;

	// Get temporary file name
	tmpnam(tmpfilename);

	// Build command
	command = strcatdyn(argv[1], false, " --input-file ", false);
	command = strcatdyn(command, true, argv[2], false);
	command = strcatdyn(command, true, " --log-file ", false);
	command = strcatdyn(command, true, tmpfilename, false);
	command = strcatdyn(command, true, " --enable-log-channel ALLOCATION ", false);
	command = redirect_stdout_to_null_device(command);

	// Run command
	if ((ret = system(command)) != 0) {
		fprintf(stderr, "Command \"%s\" exited with value %d\n", command, ret);
		exit(ret);
	}

	// Free command
	free(command);

	// Get number of memory allocations
	malloc_count = count_lines(tmpfilename);

	// For each memory allocation
	for (size_t malloc_index = 0; malloc_index < malloc_count; ++malloc_index) {
		// Print index to buffer
		sprintf(indexstrbuf, "%zu", malloc_index);

		// Build command
		command = strcatdyn(argv[1], false, " --input-file ", false);
		command = strcatdyn(command, true, argv[2], false);
		command = strcatdyn(command, true, " --malloc-failing-index ", false);
		command = strcatdyn(command, true, indexstrbuf, false);
		command = strcatdyn(command, true, " --enable-log-channel LEAKAGE", false);
		command = redirect_stdout_to_null_device(command);

		// Run command
		if ((ret = system(command)) != 0) {
			fprintf(stderr, "Command \"%s\" exited with value %d\n", command, ret);
			++fail_count;
		}

		// Free command
		free(command);
	}

	// Return fail count
	return fail_count;
}