#include <yatester/yatester.h>
#include <yatester/err.h>
#include <yatester/cmdhdl.h>
#include <yatester/runner.h>
#include <yatester/parser.h>

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include <argvp/argvp.h>

static jmp_buf env;
static yadsl_ArgvParserHandle *argvp;
static const char *input_file;
static FILE *input_fp;
#ifdef YADSL_DEBUG
static const char *log_file;
static FILE *log_fp;
#endif

static yatester_status terminate_internal(yatester_status status)
{
#ifdef YADSL_DEBUG
	size_t amb_list_size;
#endif

	yatester_terminatecmdhdl();

#ifdef YADSL_DEBUG
	if (log_fp != NULL)
	{
		yadsl_memdb_set_logger(NULL);
		fclose(log_fp);
		log_fp = NULL;
	}
#endif

	if (input_fp != NULL)
	{
		fclose(input_fp);
		input_fp = NULL;
	}

	if (argvp != NULL)
	{
		yadsl_argvp_destroy(argvp);
		argvp = NULL;
	}

#ifdef YADSL_DEBUG
	amb_list_size = yadsl_memdb_amb_list_size();

	if (amb_list_size > 0)
	{
		fprintf(stderr, "%zu items leaked\n", amb_list_size);
		return YATESTER_ERR;
	}
	else if (yadsl_memdb_error_occurred())
	{
		fprintf(stderr, "The memory debugger has detected an error\n");
		return YATESTER_ERR;
	}

	yadsl_memdb_clear_amb_list();

	if (status == YATESTER_MEM && yadsl_memdb_fail_occurred())
	{
		return YATESTER_OK;
	}
#endif

	return status;
}

static yatester_status parse_arguments_internal(int argc, char** argv)
{
#ifdef YADSL_DEBUG
	int nmatches;
	float malloc_failing_rate;
	size_t malloc_failing_index;
	unsigned int prng_seed;
	const char *log_channel_name;
#endif

	yadsl_ArgvKeywordArgumentDef kwargsdef[] =
	{
		{ "--help", 0 },
		{ "--input-file", 1 },
#ifdef YADSL_DEBUG
		{ "--log-file", 1 },
		{ "--malloc-failing-rate", 1 },
		{ "--malloc-failing-index", 1 },
		{ "--prng-seed", 1 },
		{ "--enable-log-channel", 1 },
#endif
		{ NULL, 0 }, /* End of definitions array */
	};

	argvp = yadsl_argvp_create(argc, argv);

	if (argvp == NULL)
	{
		fprintf(stderr, "Could not allocate argument vector parser\n");
		return YATESTER_MEM;
	}

	yadsl_argvp_add_keyword_arguments(argvp, kwargsdef);

	if (yadsl_argvp_has_keyword_argument(argvp, "--help"))
	{
		fprintf(stderr, "Yet Another Tester\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage: %s [options]\n", argv[0]);
		fprintf(stderr, "\n");
		fprintf(stderr, "Options:\n");
		fprintf(stderr, "--help                          Prints usage information and exits\n");
		fprintf(stderr, "--input-file <filepath>         Read commands from file instead of stdin\n");
#ifdef YADSL_DEBUG
		fprintf(stderr, "--log-file <filepath>           Write log to file instead of stderr\n");
		fprintf(stderr, "--malloc-failing-rate <rate>    Set memory allocation failing rate\n");
		fprintf(stderr, "--malloc-failing-index <index>  Set memory allocation failing index\n");
		fprintf(stderr, "--pnrg-seed <seed>              Set pseudorandom number generator seed\n");
		fprintf(stderr, "--enable-log-channel <channel>  Enable one of the following log channels:\n");
		fprintf(stderr, "                                ALLOCATION, DEALLOCATION, LEAKAGE\n");
#endif
		fprintf(stderr, "\n");
		fprintf(stderr, "Return codes:\n");
		fprintf(stderr, "0 - success\n");
		fprintf(stderr, "1 - generic error\n");
		fprintf(stderr, "2 - memory error\n");
		fprintf(stderr, "3 - input/output error\n");
		return YATESTER_ERR;
	}

	input_file = yadsl_argvp_get_keyword_argument_value(argvp, "--input-file", 0);
	
	if (input_file == NULL)
	{
		input_fp = stdin;
	}
	else
	{
		input_fp = fopen(input_file, "r");
		if (input_fp == NULL)
		{
			fprintf(stderr, "Could not open file \"%s\" in reading mode\n", input_file);
			return YATESTER_IO;
		}
	}

#ifdef YADSL_DEBUG
	log_file = yadsl_argvp_get_keyword_argument_value(argvp, "--log-file", 0);

	if (log_file == NULL)
	{
		log_fp = NULL;
	}
	else
	{
		log_fp = fopen(log_file, "w");
		if (log_fp == NULL)
		{
			fprintf(stderr, "Could not open file \"%s\" in writing mode\n", log_file);
			return YATESTER_IO;
		}
	}

	yadsl_memdb_set_logger(log_fp);

	nmatches = yadsl_argvp_parse_keyword_argument_value(argvp,	"--malloc-failing-rate", 0, "%f", &malloc_failing_rate);

	if (nmatches == 1)
	{
		yadsl_memdb_set_fail_rate(malloc_failing_rate);
	}
	else if (nmatches == 0)
	{
		fprintf(stderr, "Invalid value passed to --malloc-failing-rate\n");
		return YATESTER_ERR;
	}
	else
	{
		yadsl_memdb_set_fail_rate(0.f);
	}

	nmatches = yadsl_argvp_parse_keyword_argument_value(argvp, "--malloc-failing-index", 0, "%zu", &malloc_failing_index);

	if (nmatches == 1)
	{
		yadsl_memdb_set_fail_by_index(true);
		yadsl_memdb_set_fail_index(malloc_failing_index);
	}
	else if (nmatches == 0)
	{
		fprintf(stderr, "Invalid value passed to --malloc-failing-index\n");
		return YATESTER_ERR;
	}
	else
	{
		yadsl_memdb_set_fail_by_index(false);
	}

	nmatches = yadsl_argvp_parse_keyword_argument_value(argvp,	"--prng-seed", 0, "%u", &prng_seed);

	if (nmatches == 1)
	{
		yadsl_memdb_set_prng_seed(prng_seed);
	}
	else if (nmatches == 0)
	{
		fprintf(stderr, "Invalid value passed to --prng-seed\n");
		return YATESTER_ERR;
	}

	log_channel_name = yadsl_argvp_get_keyword_argument_value(argvp, "--enable-log-channel", 0);

	while (log_channel_name != NULL)
	{
		yadsl_MemDebugLogChannel log_channel_value;

		if (strcmp(log_channel_name, "ALLOCATION") == 0)
		{
			log_channel_value = YADSL_MEMDB_LOG_CHANNEL_ALLOCATION;
		}
		else if (strcmp(log_channel_name, "DEALLOCATION") == 0)
		{
			log_channel_value = YADSL_MEMDB_LOG_CHANNEL_DEALLOCATION;
		}
		else if (strcmp(log_channel_name, "LEAKAGE") == 0)
		{
			log_channel_value = YADSL_MEMDB_LOG_CHANNEL_LEAKAGE;
		}
		else
		{
			fprintf(stderr, "Log channel name \"%s\" is invalid\n", log_channel_name);
			return YATESTER_ERR;
		}

		yadsl_memdb_log_channel_set(log_channel_value, true);

		log_channel_name = yadsl_argvp_get_keyword_argument_value(argvp, NULL, 0);
	}
#endif

	return YATESTER_OK;
}

void assert_ok(yatester_status status)
{
	if (status != YATESTER_OK)
	{
		longjmp(env, status);
	}
}

int main(int argc, char** argv)
{
	yatester_status status = setjmp(env);

	if (status != YATESTER_OK)
	{
		goto end;
	}

	assert_ok(parse_arguments_internal(argc, argv));
	assert_ok(yatester_initializecmdhdl());
	assert_ok(yatester_parsescript(input_fp));

end:
	return terminate_internal(status);
}
