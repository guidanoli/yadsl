include(addLuaModule)
add_lua_module(luadiff luadiff diff.lua.c)
target_link_libraries(luadiff diff)