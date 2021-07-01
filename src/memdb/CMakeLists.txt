add_yadsl_library(memdb TEST
    SOURCES memdb.c memdb.h)

if(LUA_FOUND)
    add_subdirectory(lua)
endif()
