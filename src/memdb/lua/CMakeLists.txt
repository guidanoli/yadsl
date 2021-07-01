add_lua_module(luamemdb memdb memdb.lua.c)
target_link_libraries(luamemdb memdb yadsl)

