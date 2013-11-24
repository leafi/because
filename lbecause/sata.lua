
sata = {}

function sata.setup()
	local sdev = pci.get{class = 0x01, subclass = 0x06, progif = 0x01}

	if sdev then
		printf('sata')
		printf('bar1 %x bar2 %x bar3 %x', sdev.bar[1], sdev.bar[2], sdev.bar[3])
		printf('bar4 %x bar5 %x bar6 %x', sdev.bar[4], sdev.bar[5], sdev.bar[6])
	else
		print('sata device not found')
		return
	end

	local bar = sdev.bar[6]

	local cap = bc.peeki(bar)

	print('sata.lua: Current/Old IRQ: ' .. sdev.oldirq)
	print('sata.lua: Supports 64-bit Addressing: ' .. ((bit64.band(cap, 0x80000000) > 0) and 'yes' or 'no'))
	print('sata.lua: Supports Native Command Queueing: ' .. ((bit64.band(cap, 0x40000000) > 0) and 'yes' or 'no'))
	print('sata.lua: Supports AHCI mode only: ' .. ((bit64.band(cap, 0x40000) > 0) and 'yes' or 'no'))
	print('sata.lua: Number of Command Slots: ' .. (1 + bit64.band(bit64.rshift(cap, 8), 0x1f)))
	print('sata.lua: Number of Ports: ' .. (1 + bit64.band(cap, 0x1f)))
	printf('sata.lua: ACHI Version: %x', bc.peeki(bar + 0x10))

end

