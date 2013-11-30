
interrupts = {}

-- lua half of the generic interrupt handler
function call_lua_int_2(inte)
	local i = interrupts[inte]
	if i then
		for j,fun in ipairs(i) do fun() end
	end

	-- EOI?
	if inte >= 0x20 then
		if inte < 0x30 then
			bc.outb(0x20, 0x20); -- master pic EOI
			if inte >= 0x28 then
				bc.outb(0xa0, 0x20); -- slave pic EOI
			end
		end
	end
end

function interrupts.add(inte, fun)
	if not interrupts[inte] then
		interrupts[inte] = { fun }
	else
		table.insert(interrupts[inte], fun)
	end
end

function interrupts.setup()

	-- all the lua interrupts are set up in pic.c
	-- call -> lua code is handled in lua.c

	-- we do need to call bc.irqmask() to unmask things still, though.

	-- TEST
	local f = function()
		print(bc.inb(0x60))
	end
	interrupts[0x21] = { f }

end

