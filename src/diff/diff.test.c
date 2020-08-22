#include <diff/diff.h>

#include <aa/posixstring.h>
#include <stdio.h>

#include <tester/tester.h>

const char *TesterHelpStrings[] = {
	"/diff X Y    saves the difference between X and Y in slot A",
	"/equ B       asserts last value computed is equal to B",
	"/neq B      asserts last value computed is different from B",
	"/gtr B      asserts last value computed is greater than B",
	"/lwr B      asserts last value computed is lower than B",
	"/geq B      asserts last value computed is greater or equal to B",
	"/leq B      asserts last value computed is lower or equal to B",
	NULL,
};

#define matches(a, b) (strcmp(a, b) == 0)

char X[BUFSIZ], Y[BUFSIZ];
double val, eax = -1.0; /* Last computed value */

TesterReturnValue TesterInitCallback()
{
	return TESTER_OK;
}

TesterReturnValue TesterParseCallback(const char *command)
{
	if matches(command, "diff"){
		if (TesterParseArguments("ss", X, Y) != 2)
			return TESTER_ARGUMENT;
		eax = aa_utils_diff(X, Y);
		if (eax == -1.0)
			return TESTER_MALLOC;
		TesterLog("diff = %lf", eax);
	} else if matches(command, "equ") {
		if (TesterParseArguments("f", &val) != 1)
			return TESTER_ARGUMENT;
		if (!(eax == val))
			return TESTER_RETURN;
	} else if matches(command, "neq") {
		if (TesterParseArguments("f", &val) != 1)
			return TESTER_ARGUMENT;
		if (!(eax != val))
			return TESTER_RETURN;
	} else if matches(command, "gtr") {
		if (TesterParseArguments("f", &val) != 1)
			return TESTER_ARGUMENT;
		if (!(eax > val))
			return TESTER_RETURN;
	} else if matches(command, "lwr") {
		if (TesterParseArguments("f", &val) != 1)
			return TESTER_ARGUMENT;
		if (!(eax < val))
			return TESTER_RETURN;
	} else if matches(command, "geq") {
		if (TesterParseArguments("f", &val) != 1)
			return TESTER_ARGUMENT;
		if (!(eax >= val))
			return TESTER_RETURN;
	} else if matches(command, "leq") {
		if (TesterParseArguments("f", &val) != 1)
			return TESTER_ARGUMENT;
		if (!(eax <= val))
			return TESTER_RETURN;
	} else {
		return TESTER_COMMAND;
	}
	return TESTER_OK;
}

TesterReturnValue TesterExitCallback()
{
	return TESTER_OK;
}
