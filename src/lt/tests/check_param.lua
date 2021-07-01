local t = { passed = 1 }

function t.test(tparam)
	if t ~= tparam then
		error('bad self')
	end
end

return t
