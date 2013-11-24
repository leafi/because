
pci = {}
pci.devices = {}

function pci.readi(bus, slot, fun, offy)
	local address = 0x80000000
	address = bit64.bor(address, bit64.lshift(bus, 16))
	address = bit64.bor(address, bit64.lshift(slot, 11))
	address = bit64.bor(address, bit64.lshift(fun, 8))
	address = bit64.bor(address, offy)

	bc.outi(0xcf8, address)
	return bc.ini(0xcfc)
end

function pci.checkfun(bus, slot, fun)
	local devven = pci.readi(bus, slot, fun, 0)

	if devven == 0xffffffff then return end

	local stacmd = pci.readi(bus, slot, fun, 0x4)
	local classes = pci.readi(bus, slot, fun, 0x8)
	local bhlc = pci.readi(bus, slot, fun, 0xc)

	local p = {}
	local bar = {}
	p.dev = bit64.rshift(devven, 16)
	p.ven = bit64.band(devven, 0xffff)
	p.class = bit64.rshift(classes, 24)
	p.subclass = bit64.band(bit64.rshift(classes, 16), 0xff)
	p.progif = bit64.band(bit64.rshift(classes, 8), 0xff)
	p.header = bit64.band(bit64.rshift(bhlc, 16), 0xff)
	p.multifunction = (bit64.band(p.header, 0x80) == 0x80)
	p.oldirq = bit64.band(pci.readi(bus, slot, fun, 0x3c), 0xff)
	p.status = bit64.rshift(stacmd, 16)
	p.oldcmd = bit64.band(stacmd, 0xffff)

	if p.header == 0x0 then
		local ssassv = pci.readi(bus, slot, fun, 0x2c)
		p.subsys = bit64.band(bit64.rshift(ssassv, 16), 0xff)
		p.subsysven = bit64.rshift(bit64.band(ssassv, 0xff00), 16)
	elseif p.header == 0x2 then
		local ssassv = pci.readi(bus, slot, fun, 0x40)
		p.subsys = bit64.band(bit64.rshift(ssassv, 16), 0xff)
		p.subsysven = bit64.rshift(bit64.band(ssassv, 0xff00), 16)
	end

	for i = 1, 6 do
		bar[i] = pci.readi(bus, slot, fun, 0xc + 0x4 * i)
	end

	p.bar = bar
	pci.devices[table.maxn(pci.devices) + 1] = p

	if p.multifunction and (fun == 0x00) then
		for f = 1, 7 do
			pci.checkfun(bus, slot, f)
		end
	end
end

function pci.get(match)
	for k,v in pairs(pci.devices) do
		local bm = true
		for km,vm in pairs(match) do
			bm = bm and (v[km] == vm)
		end

		if bm then return v end
	end
	return nil
end

function pci.setup()
	for device = 0, 31 do
		pci.checkfun(0, device, 0)
	end
end

