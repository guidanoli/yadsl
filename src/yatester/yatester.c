#include <yatester/yatester.h>
#include <yatester/status.h>
#include <yatester/cmdhdl.h>
#include <yatester/runner.h>
#include <yatester/parser.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef YADSL_DEBUG
#include <memdb/stdlistener.h>
#endif

#include <argvp/argvp.h>

static yadsl_ArgvParserHandle *argvp;
static int is_expecting_status;
static yatester_status expected_status;
static const char *input_fname;
static FILE *input_fp;
#ifdef YADSL_DEBUG
static const char *log_fname;
static FILE *log_fp;
#endif

/**
 * @brief Initialize program
 * @param argc argument count
 * @param argv argument vector
 * @return operation status code
 */
static yatester_status initialize_internal(int argc, char** argv)
{
	int nmatches;
#ifdef YADSL_DEBUG
	size_t malloc_failing_countdown;
	const char *log_channel_name;
#endif

	static const yadsl_ArgvKeywordArgumentDef kwargsdef[] =
	{
		{ "--help", 0 },
		{ "--list-commands", 0 },
		{ "--input-file", 1 },
		{ "--expect", 1 },
#ifdef YADSL_DEBUG
		{ "--log-file", 1 },
		{ "--malloc-failing-countdown", 1 },
		{ "--enable-log-channel", 1 },
#endif
		{ NULL, 0 },
	};

#ifdef YADSL_DEBUG
	if (!yadsl_memdb_stdlistener_init())
	{
		return yatester_report(YATESTER_NOMEM, "could not add memory debugger standard listener");
	}
#endif

	argvp = yadsl_argvp_create(argc, argv);

	if (argvp == NULL)
	{
		return yatester_report(YATESTER_NOMEM, "could not allocate argument vector parser");
	}

	yadsl_argvp_add_keyword_arguments(argvp, kwargsdef);

	nmatches = yadsl_argvp_parse_keyword_argument_value(argvp, "--expect", 0, "%d", &expected_status);

	if (nmatches == 0)
	{
		return yatester_report(YATESTER_ERROR, "invalid number \"%s\" for --expect option", yadsl_argvp_get_keyword_argument_value(argvp, "--expect", 0));
	}
	else if (nmatches == 1)
	{
		is_expecting_status = 1;
	}

	input_fname = yadsl_argvp_get_keyword_argument_value(argvp, "--input-file", 0);
	
	if (input_fname == NULL)
	{
		input_fp = stdin;
	}
	else
	{
		input_fp = fopen(input_fname, "r");
		if (input_fp == NULL)
		{
			return yatester_report(YATESTER_IOERR, "%s: %s", input_fname, strerror(errno));
		}
	}

#ifdef YADSL_DEBUG
	log_fname = yadsl_argvp_get_keyword_argument_value(argvp, "--log-file", 0);

	if (log_fname == NULL)
	{
		log_fp = NULL;
	}
	else
	{
		log_fp = fopen(log_fname, "w");
		if (log_fp == NULL)
		{
			return yatester_report(YATESTER_IOERR, "%s: %s", log_fname, strerror(errno));
		}
	}

	yadsl_memdb_stdlistener_set_logger(log_fp);

	nmatches = yadsl_argvp_parse_keyword_argument_value(argvp, "--malloc-failing-countdown", 0, "%zu", &malloc_failing_countdown);

	if (nmatches == 1)
	{
		yadsl_memdb_stdlistener_set_fail_countdown(malloc_failing_countdown);
	}
	else if (nmatches == 0)
	{
		return yatester_report(YATESTER_ERROR, "invalid number \"%s\" for --malloc-failing-countdown option", yadsl_argvp_get_keyword_argument_value(argvp, "--malloc-failing-countdown", 0));
	}

	log_channel_name = yadsl_argvp_get_keyword_argument_value(argvp, "--enable-log-channel", 0);

	while (log_channel_name != NULL)
	{
		if (!yadsl_memdb_stdlistener_log_channel_set(log_channel_name, true))
		{
			return yatester_report(YATESTER_ERROR, "invalid value \"%s\" passed to --enable-log-channel option", log_channel_name);
		}

		log_channel_name = yadsl_argvp_get_keyword_argument_value(argvp, NULL, 0);
	}
#endif

	return YATESTER_OK;
}

/**
 * @brief Terminate program
 * @param status current status code
 * @return processed status code
 */
static yatester_status terminate_internal(yatester_status status)
{
	yatester_terminatecmdhdl();
	yatester_terminateparser();

#ifdef YADSL_DEBUG
	if (log_fp != NULL)
	{
		yadsl_memdb_stdlistener_set_logger(NULL);
		fclose(log_fp);
		log_fp = NULL;
	}
#endif

	if (input_fname != NULL && input_fp != NULL)
	{
		fclose(input_fp);
		input_fp = NULL;
		input_fname = NULL;
	}

	if (argvp != NULL)
	{
		yadsl_argvp_destroy(argvp);
		argvp = NULL;
	}

#ifdef YADSL_DEBUG
	if (!yadsl_memdb_stdlistener_finalize())
	{
		if (status != YATESTER_OK)
		{
			status = yatester_report(YATESTER_ERROR, "could not finalize memory debugger standard listener");
		}
	}

	if (status == YATESTER_NOMEM && yadsl_memdb_stdlistener_fail_occurred())
	{
		status = YATESTER_OK;
	}
#endif

	if (is_expecting_status)
	{
		if (status == expected_status)
		{
			status = YATESTER_OK;
		}
		else
		{
			status = yatester_report(YATESTER_ERROR, "expected and real status codes differ");
		}
	}

	return status;
}

/**
 * @brief Print command and the number of arguments it expects
 * @param command
 */
static void printcmd_cb(const yatester_command* command)
{
	int argc = command->argc;

	/* Assumes AT_MOST and AT_LEAST are involutions */
	if (argc < 0)
	{
		printf("%s expects at most %d arguments\n", command->name, AT_MOST(argc));
	}
	else
	{
		printf("%s expects at least %d arguments\n", command->name, AT_LEAST(argc));
	}
}

/**
 * @brief Run program
 * @param argc argument count
 * @param argv argument vector
 * @return status
 */
static yatester_status run_internal(int argc, char** argv)
{
	yatester_status status;

	if(status = initialize_internal(argc, argv))
	{
		return status;
	}

	if (yadsl_argvp_has_keyword_argument(argvp, "--help"))
	{
		fprintf(stderr, "Yet Another Tester\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage: %s [options]\n", argv[0]);
		fprintf(stderr, "\n");
		fprintf(stderr, "Options:\n");
		fprintf(stderr, "--help                          Print usage information and exit\n");
		fprintf(stderr, "--list-commands                 List all valid commands and exit\n");
		fprintf(stderr, "--input-file <filepath>         Read commands from file instead of stdin\n");
		fprintf(stderr, "--expect <status>               Expect the tester to return a status code\n");
#ifdef YADSL_DEBUG
		fprintf(stderr, "--log-file <filepath>           Write log to file instead of stderr\n");
		fprintf(stderr, "--malloc-failing-countdown <c>  Set memory allocation failing countdown\n");
		fprintf(stderr, "--enable-log-channel <channel>  Enable one of the following log channels:\n");
		fprintf(stderr, "                                ALLOCATION, DEALLOCATION\n");
#endif
		return YATESTER_OK;
	}

	if(status = yatester_initializeparser())
	{
		return status;
	}

	if(status = yatester_initializecmdhdl())
	{
		return status;
	}

	if (yadsl_argvp_has_keyword_argument(argvp, "--list-commands"))
	{
		yatester_itercommands(printcmd_cb);
		return YATESTER_OK;
	}

	if(status = yatester_parsescript(input_fp))
	{
		return status;
	}

	return YATESTER_OK;
}

int main(int argc, char** argv)
{
	yatester_status status;

	status = run_internal(argc, argv);

	return terminate_internal(status);
}
