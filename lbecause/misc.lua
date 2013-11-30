
-- bad busy sleep function
function sleep(ms)
	-- (timing hack based on very rough timing in virtualbox. shh...)
	ms = ms * 0.01

	while ms > 0 do
		bc.iowait() -- supposed to take 100ns (my arse it does)
		ms = ms - 0.0001
	end
end

function printf(fmt,...)
	print(string.format(fmt, ...))
end

