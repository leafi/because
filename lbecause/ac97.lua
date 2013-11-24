
-- basic AC97 audio driver

-- TODO: support ICH2..ICH5 features
-- will never todo: modem features (yeuch!!!)

ac97 = {}

function ac97.setup()
	local ichdev = pci.get{ven = 0x8086, dev = 0x2415}

	if not ichdev then
		print('ac97.lua: failed to find device. maybe you need to add your pci id?')
		print('ac97.lua: also note this driver is NOT compatible with intel hd audio')
		return
	end

	print('ac97.lua: Good news, I found your AC97 controller!')

	local MIXERBAR = bit64.band(ichdev.bar[1], 0xfffe)
	local DMABAR = bit64.band(ichdev.bar[2], 0xfffe)

	print('ac97.lua: ac97 device wants IRQ ' .. ichdev.oldirq)

	printf('ac97 MIXERBAR: 0x%x', MIXERBAR)
	printf('ac97 DMABAR: 0x%x', DMABAR)
	
	-- unmute pcm and stuff
	bc.outw(MIXERBAR + 0x2, 0)
	bc.outw(MIXERBAR + 0x4, 0)
	bc.outw(MIXERBAR + 0x6, 0)
	bc.outw(MIXERBAR + 0x18, 0)


	local dlist = bc.kmalloc(32 * 8)

	for i=dlist,dlist+32*8,8 do
		bc.pokel(i, 0)
	end
	
	-- alloc a little thing
	local sqbuf = bc.kmalloc(44100 * 2 * 2 + 1)
	if bit64.band(sqbuf, 1) > 0 then sqbuf = sqbuf + 1 end

	-- put it in the list
	bc.pokei(dlist, sqbuf)
	bc.pokew(dlist + 4, 0x8000) -- ioc
	bc.pokew(dlist + 6, 44100)
	bc.pokei(dlist + 8, sqbuf + 44100 * 2)
	bc.pokew(dlist + 12, 0x8000) -- ioc 
	bc.pokew(dlist + 14, 44100)

	-- make a square wave :D
	local k = 0xa000
	local l = 600
	for j=0,44100*2*2,2 do
		bc.pokew(sqbuf + j, k)

		if (j % l) == 0 then
			if k > 0 then
				k = 0
			else
				k = 0xa000
			end
			l = l - 1
			if l == 0 then l = 600 end
		end

	end

	-- write descriptor list
	bc.outi(DMABAR + 0x10, dlist)

	-- set last valid index
	bc.outb(DMABAR + 0x15, 1)

	-- set ready bit (yes really... wow)
	bc.outb(DMABAR + 0x1b, 1)

	-- pls something happen o pls o pls o pls

	--interrupts[0x20 + ichdev.oldirq] = { blah blah blah }
	--bc.irq_mask(ichdev.oldirq, 0)

end

