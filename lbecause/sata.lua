
sata = {}

-- TODO: ncq support, maybe? probably. at some point.
-- TODO: command completion coalescing
-- TODO: port multipliers, enclosure management. ..hot swap. some day.
--        (not soon.)

function sata.intforport(port)
	-- interrupt handler for given port
	return function()
		local portbar = sata.ports[port].portbar

		-- ok, let's interrogate PxIS
		local pxis = bc.peeki(portbar + 0x10)


	end
end

function sata.portsetup(port)
	-- sata.ncq, sata.ports, sata.bar, sata.dev, sata.addr64, sata.ncs

	-- apparently sata.ncs may be 1 for some ahci implementations. (IRL?)
	-- so should watch out for that.

	-- 'software posts new commands received by the os to empty slots in the
	--  (command) list, and sets the corresponding bit in the PxCI register.
	--  the hba continuously polls PxCI to determine if there are commands to
	--  transmit to the device.'

	local portbar = sata.bar + 0x100 + port * 0x80
	sata.ports[port].portbar = portbar

	-- setup PRDT (1k buffer, aligned to 1k)
	local prdt = bc.calloc(1024 + 1023, 1)
	prdt = prdt + 1024 - (prdt % 1024)
	bc.pokei(portbar, bit64.band(prdt, 0xffffffff))
	if sata.addr64 then bc.pokei(portbar + 0x4, bit64.rshift(prdt, 32)) end

	-- setup received FIS area (256 bytes, aligned to 256 bytes)
	local rfis = bc.calloc(256 + 255, 1)
	rfis = rfis + 256 - (rfis % 256)
	bc.pokei(portbar + 0x8, bit64.band(rfis, 0xffffffff))
	if sata.addr64 then bc.pokei(portbar + 0xc, bit64.rshift(rfis, 32)) end


	-- enable appropriate interrupts (PxIE)
	-- (todo: errors, too!)
	bc.pokei(portbar + 0x14, 0x37)

	-- program the pic a bit
	-- TODO: give it its own real interrupt... grumble grumble
	interrupts.add(0x20 + sata.dev.oldirq, sata.intforport(port))
	bc.irqmask(sata.dev.oldirq, 0)

	-- set PxCMD.FRE
	bc.pokei(portbar + 0x18, bit64.bor(bc.peeki(portbar + 0x18), 0x10))

	-- enqueue IDENTIFY command


	-- set PxCMD.ST
	bc.pokei(portbar + 0x18, bit64.bor(bc.peeki(portbar + 0x18), 0x1))


end

function sata.setup()
	local sdev = pci.get{class = 0x01, subclass = 0x06, progif = 0x01}
	sata.dev = sdev

	if sdev then
		printf('sata')
		printf('bar1 %x bar2 %x bar3 %x', sdev.bar[1], sdev.bar[2], sdev.bar[3])
		printf('bar4 %x bar5 %x bar6 %x', sdev.bar[4], sdev.bar[5], sdev.bar[6])
	else
		print('sata device not found')
		return
	end

	local bar = sdev.bar[6]
	sata.bar = bar

	local cap = bc.peeki(bar)
	local cap2 = bc.peeki(bar + 0x24)

	-- do BIOS/OS handover if necessary (i.e. if CAP2.BOH is 1)
	if (bit64.band(cap2, 1) > 0) and (bit64.band(bc.peeki(bar + 0x28, 1)) > 0) then
		print('sata.lua: Doing BIOS/OS handover (UNTESTED!)')
		-- set os ownership bit BOHC.OOS to 1
		bc.pokei(bar + 0x28, bit64.bor(bc.peeki(bar + 0x28), 2))
		-- wait for bios ownership bit BOHC.BOS to become 0
		print('sata.lua: waiting for BOHC.BOS to become 0...')
		while (bit64.band(bc.peeki(bar + 0x28), 1) > 0) do end
		-- if within 25ms BOHC.BB becomes 1, let the bios have two seconds
		print('sata.lua: okay, waiting 25ms...')
		sleep(25)

		if bit64.band(bc.peeki(bar + 0x28), 0x10) > 0 then
			print('sata.lua: ok, the bios gets two seconds to clean-up more.')
			sleep(2000)
		end

		print('sata.lua: BIOS/OS handover should be complete!')
	else
		print('sata.lua: Not doing BIOS/OS handover')
	end



	print('sata.lua: Current/Old IRQ: ' .. sdev.oldirq)
	local addr64 = bit64.band(cap, 0x80000000) > 0
	sata.addr64 = addr64
	print('sata.lua: Supports 64-bit Addressing: ' .. (addr64 and 'yes' or 'no'))
	local ncq = bit64.band(cap, 0x40000000) > 0
	sata.ncq = ncq
	print('sata.lua: Supports Native Command Queueing: ' .. (ncq and 'yes' or 'no'))

	local ahciOnly = bit64.band(cap, 0x40000) > 0
	print('sata.lua: Supports AHCI mode only: ' .. (ahciOnly and 'yes' or 'no'))

	if not ahciOnly then
		-- set GHC.AE to 1
		print('sata.lua: Set AHCI-enable...')
		bc.pokei(bar + 4, bit64.bor(bc.peeki(bar + 4), 0x80000000))
	end

	local ncs = 1 + bit64.band(bit64.rshift(cap, 8), 0x1f)
	sata.ncs = ncs
	print('sata.lua: Number of Command Slots: ' .. ncs)
	local nports = 1 + bit64.band(cap, 0x1f)
	print('sata.lua: Number of Ports: ' .. nports)
	printf('sata.lua: ACHI Version: %x', bc.peeki(bar + 0x10))

	sata.ports = {}

	local anyports = false

	-- read DET bits of SSTATUS for ports
	for port=0,(nports - 1) do
		local det = bit64.band(bc.peeki(bar + 0x100 + 0x80 * port + 0x28), 0x7)

		if det == 1 then
			printf('sata.lua: Port %d: Device present detected BUT phy not established', port)
		elseif det == 4 then
			printf('sata.lua: Port %d: Phy in offline mode! TODO: enable int', port)
		elseif det == 3 then
			printf('sata.lua: Port %d: Device detected! Will probe.', port)
			--table.insert(sata.ports, port)
			sata.ports[port] = {}
			anyports = true
		end

	end

	if not anyports then
		print('sata.lua: Didn\'t find any SATA devices! No need to continue.')
		return
	end

	for port,vt in pairs(sata.ports) do
		sata.portsetup(port)
	end
	
end

