#include <list/list.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

const char *yadsl_tester_help_strings[] = {
	"This is the interactive module of the list library",
	"You can interact with many lists at the same time",
	"Lists are identified with #, which range from 0 to 9",
	"Objects can be of any type accepted by Tester",
	"<obj> is actually <obj-type> <obj-data>",
	"All remaining lists are destroyed in the end",
	"The registered actions are the following:",
	"",
	"/create <#>",
	"/append <#> <obj>",
	"/insert <#> <idx> <obj>",
	"/remove <#> <obj>",
	"/pop <#> <idx> <expected-obj>",
	"/clear <#>",
	"/copy <#-orig> <#-dest>",
	"/count <#> <obj> <expected-count>",
	"/index <#> <obj> <expected-index>",
	"/size <#> <expected-size>",
	0,
};

yadsl_ListHandle* lists[10];

yadsl_TesterRet yadsl_tester_init()
{
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_convert_ret(yadsl_ListRet ret)
{
	switch (ret) {
	case YADSL_LIST_RET_OK:
		return YADSL_TESTER_RET_OK;
	case YADSL_LIST_RET_MEMORY:
		return YADSL_TESTER_RET_MALLOC;
	case YADSL_LIST_RET_INDEX:
		return yadsl_tester_return_external_value("index");
	case YADSL_LIST_RET_NOT_FOUND:
		return yadsl_tester_return_external_value("not found");
	default:
		return yadsl_tester_return_external_value("unknown");
	}
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	yadsl_ListRet ret = YADSL_LIST_RET_OK;
	if (yadsl_testerutils_match(command, "create")) {
	} else if (yadsl_testerutils_match(command, "append")) {
	} else if (yadsl_testerutils_match(command, "insert")) {
	} else if (yadsl_testerutils_match(command, "remove")) {
	} else if (yadsl_testerutils_match(command, "pop")) {
	} else if (yadsl_testerutils_match(command, "clear")) {
	} else if (yadsl_testerutils_match(command, "copy")) {
	} else if (yadsl_testerutils_match(command, "count")) {
	} else if (yadsl_testerutils_match(command, "index")) {
	} else if (yadsl_testerutils_match(command, "size")) {
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return yadsl_tester_convert_ret(ret);
}

yadsl_TesterRet yadsl_tester_release()
{
	for (int i = 0; i < sizeof(lists)/sizeof(*lists); ++i)
		if (lists[i])
			yadsl_list_destroy(lists[i], yadsl_tester_object_free);
	return YADSL_TESTER_RET_OK;
}
