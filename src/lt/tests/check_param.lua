local t = {}

function t.test(tparam)
	print('my test called')
	if t ~= tparam then
		error('bad self')
	end
end

return t
