local t = { errors = {}, passed = 7, failed = 13 }

for i = 1, t.passed + t.failed do
	local func
	if i <= t.passed then
		func = function() end
	else
		local err = tostring(i*i)
		func = function() error(err) end
		t.errors['test' .. i] = err
	end
	t['test' .. i] = func
end

return t
