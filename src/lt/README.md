# lt - Lua Tester

Lua Tester is a (generic) C library meant for testing other C libraries through test cases specified in Lua.
It is meant to be linked against another (specific) C library that defines the functions exported to the Lua environment.
The end result is an executable that runs all the test cases specified in Lua and prints diagnostics.

Example of Lua input:

```lua
local tests = {}

function tests.testcase(m)
	assert(m.reverse(123) == 321)
	assert(m.isodd(54) == False)
end

return tests
```

Example of C code:

```c
#include "lauxlib.h"

static int lua_reverse(lua_State* L) {...}
static int lua_isodd(lua_State* L) {...}

const luaL_Reg lt_lib[] = {
	{"reverse", lua_reverse},
	{"isodd",   lua_isodd},
	{NULL,      NULL},
};
```

## Advantages of using lt over tester and yatester

It allows for segregating the test script into isolated test cases
* Lua is lexically scoped
* Lua functions work like closures
* Lua functions can be called in "protected" mode

It gives more power and flexiblity to the tester with a full-blown programming language
* Lua is much more than an imperative language (like `tester` and `yatester` script language)
* Lua has plenty of built-in functionalities that can help testers in complex or repetitive tasks
* Lua functions are first-class citizens and can be called in "protected" mode (for error handling)
* Lua is a popular scripting language. This eliminates the need to learn a new language for some

It gives more power and flexiblity to the C programmer too
* Lua functions can accept and return any number of values
* Lua values can be more complex, like tables, userdata, threads and functions.
* Lua errors can be raised and caught in any protected environment
* Lua tables have constant lookup time (faster than linear lookup time in `tester`)

## Disadvantages of using lt over tester and yatester

It adds Lua as one more dependency (although very lightweight?).
It requires the tester to know how to code Lua (although very simple?)
It adds a new layer for errors (although very stable?)
