
-- virtio-net network card interface

virtionet = {}

function virtionet.setup()
	local dev = pci.get{ven = 0x1af4, subsys = 0x01}

	if not dev then
		print('virtionet.lua: device not found')
		return
	end

	if not ((dev.dev >= 0x1000) and (dev.dev <= 0x103f)) then
		print('virtionet.lua: found DeviceID not between 0x1000 and 0x103f.')
		print('virtionet.lua: this is probably wrong... fix driver pls')
		return
	end

	local iobar = bit64.band(dev.bar[1], 0xfffffffc)
	local caps = bc.ini(iobar + 0)

	printf('virtionet.lua: Device features: %x', caps)

	-- VIRTIO_NET_F_MAC
	local caps_mac = bit64.band(caps, 0x20) > 0
	print('virtionet.lua: VIRTIO_NET_F_MAC: ' .. (caps_mac and 'yes' or 'no'))
	if not caps_mac then
		print('Welp; device doesn\'t set a MAC itself. Creepy.')
	else
		printf('virtionet.lua: Physical MAC address %x-%x-%x-%x-%x-%x', bc.inb(iobar + 0x14), 
						bc.inb(iobar + 0x15), bc.inb(iobar + 0x16), bc.inb(iobar + 0x17),
						bc.inb(iobar + 0x18), bc.inb(iobar + 0x19))
	end
	printf('virtionet.lua: setting up device...')

	local DEVICE_STATUS = iobar + 18

	-- reset device, set acknowledge bit & set driver
	bc.outb(DEVICE_STATUS, 0)
	bc.outb(DEVICE_STATUS, 0x1) -- ACKNOWLEDGE
	bc.outb(DEVICE_STATUS, 0x3) -- ACKNOWLEDGE, DRIVER

	-- TODO: offload checksum feature
	-- TODO: offload packet segmentation feature
	-- TODO: maybe merge rxbuf, maybe mq
	-- TODO: control queue stuff? (set promiscuous mode, set mac addr, ...)
	-- TODO: ... maybe msi-x i guess ...

	local QUEUE_SELECT = iobar + 14
	local QUEUE_SIZE = iobar + 12
	local QUEUE_ADDRESS = iobar + 8

	-- get queue size for virtqueue 0 (receive), 1 (transmit)
	-- 

	local setup_queue = function(idx)
		-- write index to Queue Select
		bc.outw(QUEUE_SELECT, idx)
		-- read size from Queue Size field (should NOT be 0)
		local qsz = bc.inw(QUEUE_SIZE)
		if qsz == 0 then
			printf('empty queue %d! should NOT be empty', idx)
			return
		end

		printf('queue %d has %d elems', idx, qsz)

		-- allocate memory for queue & write physical addr to Queue Address
		-- queue mem must be 4k-aligned. (and a couple of other rules apply too)
		-- we can't do a little kmalloc before-hand to get us 4k-aligned then go
		--  straight ahead, because lua can malloc without our consent.
		-- so, we take a slightly (but not really) wasteful approach instead.

		-- size formula: (align) + 16*qsz + 2*3 + 2*qsz + (align) + 2*3 + 8*qsz
		local actual_size = 16 * qsz + 6 + 2 * qsz
		actual_size = actual_size + 4096 - (actual_size % 4096) -- align
		actual_size = actual_size + 6 + 8 * qsz
		printf('actual predicted size in bytes: %d', actual_size)

		-- deal with initial align
		local addr = bc.kmalloc(actual_size + 4095)
		addr = addr + 4096 - (addr % 4096)
		printf('going to allocate at 0x%x', addr)

		-- set up address
		bc.outi(QUEUE_ADDRESS, bit64.rshift(addr, 12))
		return {addr = addr, qsz = qsz}
	end

	local rxq = setup_queue(0)
	local txq = setup_queue(1)
	
	-- queue layout:
	--   vring_desc desc[qsz]   (16 bytes each)
	--   vring_avail avail      (ring of available descriptor heads)
	--   (padding)
	--   vring_used used        (ring of used descriptor heads)



	-- write supported features todevice (MAC if supported)
	-- set DRIVER_OK
	--bc.outb(iobar + 4, 0x7) -- ACKNOWLEDGE, DRIVER, DRIVER_OK
end

