return {
	testok = function() print('ok called') end,
	testfail = function() print('fail also called') error('my error message') end,
}
