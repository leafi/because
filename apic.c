extern unsigned char inb(unsigned short port);
extern unsigned int ind(unsigned short port);
extern unsigned short inw(unsigned short port);

extern void outb(unsigned short port, unsigned char val);
extern void outd(unsigned short port, unsigned int val);
extern void outw(unsigned short port, unsigned short val);

extern void printki(long n);
extern void printk(char* s);

extern void intq();
extern void testq();

extern void disable_pic();
extern void enable_ioapic_maybe();

extern void disable_int();
extern void enable_int();

unsigned int* ioapic1;

void write_ioapic(unsigned int regsel, unsigned int regwin)
{
	*ioapic1 = regsel;
	*(ioapic1 + 0x10) = regwin;
}

unsigned int read_ioapic(unsigned int regsel)
{
	*(unsigned int*)(ioapic1) = regsel;
	//printk("-- ");
	//printki(((void*)ioapic1) + 0x10);
	//printk(" --\n");
	return *(unsigned int*)(((void*)ioapic1) + 0x10);   // 10);
}

void setup_apic(char really)
{
	//disable_pic();

	char ioapicnum = *((char*)0x5031);
	printk("IOAPICNUM: ");
	printki(ioapicnum);
	printk("\n");

	int j; int k;

	if (ioapicnum > 0) {
		printk("looking at i/o apic #1:\n");
		ioapic1 = *((unsigned int*)0x5068);
		printk("base addr ");
		printki((unsigned int) ioapic1);
		printk("\n");

		printk("IOAPICID: ");
		printki(read_ioapic(0x0));
		printk("\nIOAPICVER: ");
		printki(read_ioapic(0x1));
		printk("\n");

		for (j = 0; j < 24; j++) {
			printk("IRQ ");
			printkid(j);
			printk(": ");

			k = 0x10 + 2 * j;

			printki((((long)read_ioapic(k + 1)) << 32) + ((long)read_ioapic(k)));

			if ((j % 4) == 3) {
				printk("\n");
			} else {
				printk(" ");
			}
		}
		//printk("0x11:0x10 :: ");
		//printki((((long)read_ioapic(0x11)) << 32) + ((long)read_ioapic(0x10)));
		//printk("\n");

	} else {
		printk("No I/O APICs found.  Not setting them up, obv.\n");
		printk("If boot fails retry with an i/o apic\n");
	}

	printk("---------------------\n");
	printk("Local APIC setup tiem\n");
	printk("Spurious Interrupt Vector Register: ");
	printki(*(unsigned int*)(0xfee000f0));
	printk("\nLocal APIC Version Register: ");
	unsigned int localApicVersion = *(unsigned int*)(0xfee00030);
	unsigned int version = localApicVersion & 0xff;
	unsigned int maxLvtEntry = (localApicVersion >> 16) & 0xff;
	printki(*(unsigned int*)(0xfee00030));
	printk("\n");
	printk(" ^ version: ");
	printki(version);
	printk(" maxLvtEntry minus one: ");
	printkid(maxLvtEntry);
	printk("\n");
	printk("LVT LINT0: ");
	printki(*(unsigned int*)(0xfee00350));
	printk("\n");
	printk("LVT LINT1: ");
	printki(*(unsigned int*)(0xfee00370));
	printk("\n");

	printk("int 0: ");
	printki(*(unsigned long*)(0x0));
	printk(" ");
	printki(*(unsigned long*)(0x8));
	printk("\nint 1: ");
	printki(*(unsigned long*)(0x10));
	printk(" ");
	printki(*(unsigned long*)(0x18));
	printk("\nint 33: ");
	printki(*(unsigned long*)(0x10 * 33));
	printk(" ");
	printki(*(unsigned long*)(0x10 * 33 + 0x8));
	printk("\nint 231: ");
	printki(*(unsigned long*)(0x10 * 230));
	printk(" ");
	printki(*(unsigned long*)(0x10 * 230 + 0x8));
	printk("\n");

	// set up int 0x80
	printk("setting up int 0x80\n");
	unsigned long mn = (unsigned long)(&intq);

	unsigned long p = (mn & 0xffff0000);
	printk("p: ");
	printki(p);
	p = p >> 16;
	printk("\np: ");
	printki(p);
	printk("\n");
	printk("p<<48: ");
	printki(p << 48);
	printk("\n");
	unsigned long l = ((mn & 0xffff) + (p << 48) + 0x80000 + 0x8f0000000000);
	printki(l);
	printk("\n");
	//int x = 3 / 0;
	printk("writing to ");
	printki((unsigned long)(unsigned long*)(0x10 * 0x7f));
	printk("\n");
	//*(unsigned long*)(0x10 * 0x80) = l;
	*(unsigned long*)(0x10 * 0x1) = l;
	*(unsigned long*)(0x10 * 33) = l;
	*(unsigned long*)(0x10 * 231) = l;
	*(unsigned long*)(0x10 * 0x80) = l;
	int x;
	for (x = 0; x < 256; x++) {
		*(unsigned long*)(0x10 * x) = l;
	}
	/*printk(" - ");
	printki(*(unsigned long*)(0));
	printk("\n - ");
	*(unsigned long*)(0) = l;
	printki(*(unsigned long*)(0));
	printk("\n");*/

	printk("intq: ");
	printki((unsigned long)(&intq));
	printk("\n");

	*(char*)(0x200000) = 0;

	printk("apic id for cpu core 0 ");
	printki(*(char*)(0x5100));
	printk("\n");

	printk("PIC1_DATA ");
	printki(inb(0x21));
	printk("\n");
	printk("PIC2_DATA ");
	printki(inb(0xa1));
	printk("\n");

	// may as well. why the hell not(!)
	outb(0x21, 0);
	outb(0xa1, 0);

	enable_ioapic_maybe();

	enable_int();

	printk("the msr thing says: ");
	readthemsrthing();
	printki(*(unsigned int*)(0x200000));
	printk(" ");
	printki(*(unsigned int*)(0x200004));
	printk("\n");

	// do test int 0x80
	testq();
	testq();

	/*printk("in 0x200000: ");
	printki(*(char*)(0x200000));
	printk("\n");*/

	for (j = 0; j < 24; j++) {
		k = 0x10 + 2 * j;

		//printki((((long)read_ioapic(k + 1)) << 32) + ((long)read_ioapic(k)));
		write_ioapic(k, 0x21);
		write_ioapic(k + 1, 0);
	}
	
}

void setup_apic_notreally()
{
	int j; int k;
	printk("Setting up ioapic (but not really)\n");
	for (j = 0; j < 24; j++) {
		k = 0x10 + 2 * j;

		//printki((((long)read_ioapic(k + 1)) << 32) + ((long)read_ioapic(k)));
		write_ioapic(k, 0x21);
		write_ioapic(k + 1, 0);
	}
}
	

