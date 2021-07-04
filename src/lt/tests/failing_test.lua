local t = { errors = {} }

for i = 1, 20 do
	local func
	local name = 'test' .. i
	if i <= 10 then
		func = function() end
	else
		local err = tostring(i*i)
		func = function() error(err) end
		t.errors[name] = err
	end
	t[name] = func
end

return t
