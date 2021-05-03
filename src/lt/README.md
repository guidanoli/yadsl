# lt - Lua Tester

Lua Tester is an auxiliary program for testing Lua modules.
You may execute it with the following arguments:

```
$ lt [script]
```

Where `script` is the path to a Lua script file.
If ommitted, the program reads from the standard input, instead.
The script should return a table.
The program traverses this table, calling every function whose key is either a string or a number,
and passes the table itself as argument as if it were called with the `:` syntax.

An example of Lua script:

```lua
local mymodule = require "mymodule"

local tests = {}

function tests:test_odds_and_evens()
	for i = 1, 10 do
		if i % 2 == 0 then
			assert(mymodule.iseven(i))
		else
			assert(mymodule.isodd(i))
		end
	end
end

function tests:test_error()
	error('my error message')
end

return tests
```

An example of output:

```
test_odds_and_evens ... ok
test_error ... FAIL

FAIL: test_error
stdin:1: my error message
stack traceback:
	[C]: in function 'error'
	stdin:1: in function <stdin:1>
	[C]: in ?

lt: 1 failed, 1 passed
```

## Advantages of using lt over tester

It allows for segregating the test script into isolated test cases
* Lua is lexically scoped
* Lua functions work like closures

It provide more power and flexiblity to the tester with a full-blown programming language
* Lua is much more than an imperative language (like `tester` script language)
* Lua has plenty of built-in functionalities that can help testers in complex or repetitive tasks
* Lua functions are first-class citizens and can be called in "protected" mode (for error handling)
* Lua is a popular scripting language. This eliminates the need to learn a new language for some

It provides more power and flexiblity to the C programmer too
* Lua functions can accept and return any number of values
* Lua values can be more complex, like tables, userdata, threads and functions.
* Lua errors can be raised and caught in any protected environment
* Lua tables have constant lookup time (faster than linear lookup time in `tester`)

## Disadvantages of using lt over tester

* It adds Lua as one more dependency (although very lightweight?).
* It requires previous knowledge of Lua (although very simple?)
* It adds a new layer for errors (although very stable?)
