#include <time.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <signal.h>

#include <yadsl/utl.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

/**
 * @brief Memory Failing Simulator
 * This program intends to simulate memory allocation failure by
 * passing special arguments to the tester module that configure
 * the Memory Debugger in such a way that certain calls to malloc,
 * realloc and calloc will fail
 * Usage:
 * 
 * memfail <tester-module-file-path> <tester-script>
 * 
 * Returns the number of tests that have failed.
*/

static const char* programname; /**< Program name */
static volatile bool interrupted; /**< Interrupt flag */
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

static void interrupt_handler(int sig)
{
	interrupted = true;
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
		fprintf(stderr, "%s: bad malloc in strcatdyn\n", programname);
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
		fprintf(stderr, "%s: could not open %s\n", programname, filename);
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
	char randnumstrbuf[YADSL_BUFSIZ_FOR(unsigned long)];
	char* filename;
	sprintf(randnumstrbuf, "%lx", num);
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
	bool fail_by_countdown;
	size_t malloc_countdown;
	bool log_allocation;
	bool log_deallocation;
}
tester_arguments;

/**
 * @brief Runs tester with different parameters
 * @param fail_by_countdown fail by countdown
 * @param malloc_countdown memory allocation failing countdown
 * @param log_allocation log allocation to log file
 * @return log file line count
*/
static void run_tester(tester_arguments* args)
{
	int ret;

	// Free previous command
	if (command != NULL)
		free(command);

	// Build command
	command = strcatdyn(tester, false, " --input-file ", false);
	command = strcatdyn(command, true, script, false);
	command = strcatdyn(command, true, " --log-file ", false);
	command = strcatdyn(command, true, tmpfilename, false);

	if (args->fail_by_countdown) {
        char subcommand[26 + YADSL_BUFSIZ_FOR(size_t)];
		sprintf(subcommand, " --malloc-failing-countdown %zu", args->malloc_countdown);
		command = strcatdyn(command, true, subcommand, false);
	}
	
	if (args->log_allocation)
		command = strcatdyn(command, true, " --enable-log-channel ALLOCATION", false);
	
	if (args->log_deallocation)
		command = strcatdyn(command, true, " --enable-log-channel DEALLOCATION", false);
	
	// Run command
	if ((ret = system(command)) != 0) {
		fprintf(stderr, "%s: command \"%s\" exited with value %d\n", programname, command, ret);
		release_resources();
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char** argv)
{
	unsigned long hash = 0;
	int fail_count = 0;
	size_t malloc_count;
	tester_arguments args;
	time_t now;

	// Get program name
	if (argc > 0 && argv[0][0] != '\0') {
		programname = argv[0];
	} else {
		programname = "memfail";
	}

	// Check argument count
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <tester> <script>\n", programname);
		release_resources();
		exit(EXIT_FAILURE);
	}

	// Set signal handler
	signal(SIGINT, interrupt_handler);

	// Store arguments
	tester = argv[1];
	script = argv[2];

	// Hash all arguments
	for (int i = 0; i < argc; ++i)
		hash ^= get_string_digest(argv[i]);

	// Hash current time
	// (source of randomness)
	now = time(NULL);
	hash ^= get_string_digest(ctime(&now));
	
	// Get filename
	tmpfilename = get_filename(hash);

	// Get number of memory allocations
	args = (tester_arguments) { .log_allocation = true };
	run_tester(&args);
	malloc_count = count_lines(tmpfilename);

	fprintf(stderr, "%s: %zu memory allocations identified\n", programname, malloc_count);
	
	// Run program and failing each memory allocation
	for (size_t malloc_countdown = 1; malloc_countdown <= malloc_count && !interrupted; ++malloc_countdown) {
		fprintf(stderr, "%s: Testing failing memory allocation %zu/%zu\n", programname, malloc_countdown, malloc_count);
		args = (tester_arguments) { .fail_by_countdown = true,
		                            .malloc_countdown = malloc_countdown };
		run_tester(&args);
	}

	// Release resources
	release_resources();

	if (interrupted)
	{
		// Exit with failure
		fprintf(stderr, "%s: Interrupted\n", programname);
		return EXIT_FAILURE;
	}
	else
	{
		// Exit with success
		return EXIT_SUCCESS;
	}
}
