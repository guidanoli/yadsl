return {
	ok = function() print('ok called') end,
	fail = function() print('fail also called') error('my error message') end,
}
