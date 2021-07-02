#include <yadsl/dll.h>
#include <memdb/lua/memdb.h>

YADSL_EXPORT int luaopen_memdb(lua_State* L)
{
	yadsl_memdb_openlib(L);
	return 1;
}
