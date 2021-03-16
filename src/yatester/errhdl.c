#include <yatester/errhdl.h>
#include <yatester/builtins.h>

static yatester_status expected_status[2];

void yatester_builtin_expect(const char** argv)
{
	expected_status[1] = YATESTER_ERROR;
}

yatester_status yatester_evaluatestatus(yatester_status status)
{
	if (status == expected_status[0])
	{
		expected_status[0] = expected_status[1];
		expected_status[1] = YATESTER_OK;
		return YATESTER_OK;
	}
	else if (status == YATESTER_OK)
	{
		return YATESTER_NOERROR;
	}
	else
	{
		return status;
	}
}
