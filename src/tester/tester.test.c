#include <yadsl/posixstring.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

char buffer[BUFSIZ];

const char *yadsl_tester_help_strings[] = {
	"/help                            Prints help strings, just like this one",
	"/print [s|i|f] <text>            Prints text formated as string, integer or float",
	"/cmp-pld <dtype> <data>          Compares data with preloaded data (see Pre-loaded data below)",
	"/obj-parse <dtype> <data> <slot> Parses object and stores in slot (0 from 9)",
	"/obj-free <slot>                 Free object in a given slot",
	"/obj-copy <src> <dest>           Copy object from one slot to another",
	"/obj-eq <slot1> <slot2> <eq?>    Check if two objects are equal or not",
	"/throw                           Throws custom error",
	"",
	"Preloaded data",
	"i 42",
	"l 2020",
	"f 3.14",
	"c *",
	"s \"lorem ipsum\"",
	"z 9001",
	NULL,
};

void* objects[10];

yadsl_TesterRet yadsl_tester_init()
{
	puts("TesterInitCallback called");
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	if (yadsl_testerutils_match(command, "help")) {
		yadsl_tester_print_help_strings();
	} else if (yadsl_testerutils_match(command, "print")) {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (strlen(buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (yadsl_testerutils_match(buffer, "s")) {
			if (yadsl_tester_parse_arguments("s", buffer) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			yadsl_tester_log("String: '%s'", buffer);
		} else if (yadsl_testerutils_match(buffer, "i")) {
			int i;
			if (yadsl_tester_parse_arguments("i", &i) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			yadsl_tester_log("Integer: '%d'", i);
		} else if (yadsl_testerutils_match(buffer, "l")) {
			long l;
			if (yadsl_tester_parse_arguments("l", &l) != 1)
				return YADSL_TESTER_RET_RETURN;
			yadsl_tester_log("Long: '%ld'", l);
		} else if (yadsl_testerutils_match(buffer, "f")) {
			float f;
			if (yadsl_tester_parse_arguments("f", &f) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			yadsl_tester_log("Float: '%f'", f);
		} else if (yadsl_testerutils_match(buffer, "z")) {
			size_t z;
			if (yadsl_tester_parse_arguments("z", &z) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			yadsl_tester_log("Size Type: '%zu'", z);
		} else if (yadsl_testerutils_match(buffer, "c")) {
			char c;
			if (yadsl_tester_parse_arguments("c", &c) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			yadsl_tester_log("Character: '%c'", c);
		} else {
			return YADSL_TESTER_RET_ARGUMENT;
		}
	} else if (yadsl_testerutils_match(command, "log")) {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		yadsl_tester_log(buffer);
	} else if (yadsl_testerutils_match(command, "throw")) {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		return yadsl_tester_return_external_value(buffer);
	} else if (yadsl_testerutils_match(command, "cmp-pld")) {
		char dtype;
		if (yadsl_tester_parse_arguments("c", &dtype) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		switch (dtype) {
		case 'i':
		{
			int i;
			if (yadsl_tester_parse_arguments("i", &i) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			if (i != 42)
				return YADSL_TESTER_RET_ARGUMENT;
		}
			break;
		case 'l':
		{
			long l;
			if (yadsl_tester_parse_arguments("l", &l) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			if (l != 2020)
				return YADSL_TESTER_RET_ARGUMENT;
		}
			break;
		case 'f':
		{
			float f;
			if (yadsl_tester_parse_arguments("f", &f) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			if (fabsf(f - 3.14f) > 1e-5)
				return YADSL_TESTER_RET_ARGUMENT;
		}
			break;
		case 'c':
		{
			char c;
			if (yadsl_tester_parse_arguments("c", &c) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			if (c != '*')
				return YADSL_TESTER_RET_ARGUMENT;
		}
			break;
		case 's':
			if (yadsl_tester_parse_arguments("s", buffer) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			if (strcmp(buffer, "lorem ipsum") != 0)
				return YADSL_TESTER_RET_ARGUMENT;
			break;
		case 'z':
		{
			size_t z;
			if (yadsl_tester_parse_arguments("z", &z) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			if (z != 9001)
				return YADSL_TESTER_RET_ARGUMENT;
		}
			break;
		}
	} else if (yadsl_testerutils_match(command, "obj-parse")) {
		void* obj = yadsl_tester_object_parse();
		if (obj == NULL)
			return YADSL_TESTER_RET_MALLOC;
		size_t slot;
		if (yadsl_tester_parse_arguments("z", &slot) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (slot >= sizeof(objects) / sizeof(*objects))
			return YADSL_TESTER_RET_ARGUMENT;
		if (objects[slot]) {
			yadsl_tester_object_free(objects[slot]);
			objects[slot] = NULL;
		}
		objects[slot] = obj;
	} else if (yadsl_testerutils_match(command, "obj-free")) {
		size_t slot;
		if (yadsl_tester_parse_arguments("z", &slot) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (slot >= sizeof(objects) / sizeof(*objects))
			return YADSL_TESTER_RET_ARGUMENT;
		if (!objects[slot])
			return YADSL_TESTER_RET_ARGUMENT;
		yadsl_tester_object_free(objects[slot]);
	} else if (yadsl_testerutils_match(command, "obj-copy")) {
		size_t src, dest;
		if (yadsl_tester_parse_arguments("zz", &src, &dest) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		if (src >= sizeof(objects) / sizeof(*objects))
			return YADSL_TESTER_RET_ARGUMENT;
		if (dest >= sizeof(objects) / sizeof(*objects))
			return YADSL_TESTER_RET_ARGUMENT;
		if (src == dest)
			return YADSL_TESTER_RET_ARGUMENT;
		if (objects[dest]) {
			yadsl_tester_object_free(objects[dest]);
			objects[dest] = NULL;
		}
		objects[dest] = yadsl_tester_object_copy(objects[src]);
		if (objects[dest] == NULL)
			return YADSL_TESTER_RET_MALLOC;
	} else if (yadsl_testerutils_match(command, "obj-eq")) {
		size_t slot1, slot2;
		if (yadsl_tester_parse_arguments("zzs", &slot1, &slot2, &buffer) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		if (slot1 >= sizeof(objects) / sizeof(*objects))
			return YADSL_TESTER_RET_ARGUMENT;
		if (slot2 >= sizeof(objects) / sizeof(*objects))
			return YADSL_TESTER_RET_ARGUMENT;
		if (!objects[slot1] || !objects[slot2])
			return YADSL_TESTER_RET_ARGUMENT;
		bool expected = yadsl_testerutils_str_to_bool(buffer);
		bool obtained = yadsl_tester_object_equal(objects[slot1], objects[slot2]);
		if (expected != obtained)
			return YADSL_TESTER_RET_ARGUMENT;
;	} else {
		return YADSL_TESTER_RET_COMMAND;
	}
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_release()
{
	puts("TesterExitCallback called");
	for (size_t i = 0; i < sizeof(objects) / sizeof(*objects); ++i)
		if (objects[i])
			yadsl_tester_object_free(objects[i]);
	return YADSL_TESTER_RET_OK;
}
