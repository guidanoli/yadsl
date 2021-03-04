#include <%name%/%name%.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

const char *yadsl_tester_help_strings[] = {0};

yadsl_TesterRet yadsl_tester_init()
{
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	return YADSL_TESTER_RET_COUNT;
}

yadsl_TesterRet yadsl_tester_release()
{
	return YADSL_TESTER_RET_OK;
}
