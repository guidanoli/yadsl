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

static char* tmpfilename; /**< Temporary file name */
static char* command; /**< Command */
static char* tester; /**< Tester */
static char* script; /**< Script */

/**
 * @brief Release static resources
*/
static void release_resources()
{
	if (command)
		free(command);

	if (tmpfilename) {
		remove(tmpfilename);
		free(tmpfilename);
	}
}

/**
 * @brief Concatenates two strings dynamically
 * @param a first string
 * @param is_a_dyn if a can be deallocated after concatenation
 * @param b second string
 * @param is_b_dyn if b can be deallocated after concatenation
 * @return newly concatenated string or NULL
*/
static char* strcatdyn(char* a, bool is_a_dyn, char* b, bool is_b_dyn)
{
	char* s = malloc(strlen(a) + strlen(b) + 1);
	if (s == NULL) {
		fprintf(stderr, "Bad malloc in strcatdyn\n");
		release_resources();
		exit(EXIT_FAILURE);
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
static size_t count_lines(const char* filename)
{
	FILE* f = fopen(filename, "r");
	size_t linecnt = 0;
	if (f == NULL) {
		fprintf(stderr, "Could not open %s\n", filename);
		release_resources();
		exit(EXIT_FAILURE);
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
 * @brief Get file name based on number
 * @param num file number
 * @return dynamically allocated file name
*/
static char* get_filename(unsigned long num)
{
	char randnumstrbuf[12];
	char* filename;
	sprintf(randnumstrbuf, "%x", num);
	filename = strcatdyn("memfail_", false, randnumstrbuf, false);
	filename = strcatdyn(filename, true, ".tmp", false);
	return filename;
}

static unsigned long get_string_digest(char* str)
{
	unsigned long hash = 5381;
	char c;
	while (c = *str++)
		hash = ((hash << 5) + hash) + (unsigned long) c; /* hash * 33 + c */
	return hash;
}

/**
 * @brief Tester arguments
*/
typedef struct
{
	bool fail_by_index;
	size_t malloc_index;
	bool log_allocation;
	bool log_deallocation;
	bool log_leakage;
}
tester_arguments;

/**
 * @brief Runs tester with different parameters
 * @param fail_by_index fail by index
 * @param malloc_index memory allocation failing index
 * @param log_allocation log allocation to log file
 * @param log_leakage log leakage to log file
 * @return log file line count
*/
static void run_tester(tester_arguments* args)
{
	int ret;
	char subcommand[64];

	// Free previous command
	if (command != NULL)
		free(command);

	// Build command
	command = strcatdyn(tester, false, " --input-file ", false);
	command = strcatdyn(command, true, script, false);
	command = strcatdyn(command, true, " --log-file ", false);
	command = strcatdyn(command, true, tmpfilename, false);

	if (args->fail_by_index) {
		sprintf(subcommand, " --malloc-failing-index %zu", args->malloc_index);
		command = strcatdyn(command, true, subcommand, false);
	}
	
	if (args->log_allocation)
		command = strcatdyn(command, true, " --enable-log-channel ALLOCATION", false);
	
	if (args->log_deallocation)
		command = strcatdyn(command, true, " --enable-log-channel DEALLOCATION", false);
	
	if (args->log_leakage)
		command = strcatdyn(command, true, " --enable-log-channel LEAKAGE", false);

	// Run command
	if ((ret = system(command)) != 0) {
		fprintf(stderr, "Command \"%s\" exited with value %d\n", command, ret);
		release_resources();
		exit(ret);
	}
}

int main(int argc, char** argv)
{
	unsigned long hash = 0;
	int fail_count = 0;
	size_t malloc_count;
	tester_arguments args;

	// Check argument count
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <tester> <script>\n", argv[0]);
		release_resources();
		exit(EXIT_FAILURE);
	}

	// Store arguments
	tester = argv[1];
	script = argv[2];

	// Hash all arguments
	for (int i = 0; i < argc; ++i)
		hash ^= get_string_digest(argv[i]);

	// Get filename
	tmpfilename = get_filename(hash);

	// Check for memory leaks without memory failing
	args = (tester_arguments) { .log_leakage = true };
	run_tester(&args);

	// Get number of memory allocations
	args = (tester_arguments) { .log_allocation = true };
	run_tester(&args);
	malloc_count = count_lines(tmpfilename);
	
	// Check for memory leaks with memory failing
	for (size_t malloc_index = 0; malloc_index < malloc_count; ++malloc_index) {
		args = (tester_arguments) { .fail_by_index = true,
		                            .malloc_index = malloc_index,
		                            .log_leakage = true };
		run_tester(&args);
	}

	// Release resources
	release_resources();

	// Exit with success
	return EXIT_SUCCESS;
}
