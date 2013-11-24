extern unsigned char inb(unsigned short port);
extern unsigned int ind(unsigned short port);
extern unsigned short inw(unsigned short port);

extern void outb(unsigned short port, unsigned char val);
extern void outd(unsigned short port, unsigned int val);
extern void outw(unsigned short port, unsigned short val);

extern void printki(long n);
extern void printk(char* s);

extern void disable_local_apic();

extern void disable_int();
extern void enable_int();

extern void io_wait();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

void pic_remap(int offset1, int offset2);
void irq_mask(unsigned char irq, unsigned char mask);
void set_int(unsigned char inte, void* funptr);

void setup_pic()
{
	disable_local_apic();
	disable_int();

	// remap int 0x21 (is irq 0x1)
	/*unsigned long mn = (unsigned long)(&intqold);
	unsigned long p = (mn & 0xffff0000);
	p >>= 16;
	unsigned long l = ((mn & 0xffff) + (p << 48) + 0x80000 + 0x8f0000000000);
	*(unsigned long*)(0x10 * 0x21) = l;*/
	
	pic_remap(0x20, 0x28);

	// setup non-0x20+1, non-0x20+12 irqs pls
	set_int(0x20, irq0);
	//set_int(0x21, irq1);
	//set_int(0x22, irq2);
	set_int(0x23, irq3);
	set_int(0x24, irq4);
	set_int(0x25, irq5);
	set_int(0x26, irq6);
	set_int(0x27, irq7);
	set_int(0x28, irq8);
	set_int(0x29, irq9);
	set_int(0x2a, irq10);
	set_int(0x2b, irq11);
	//set_int(0x2c, irq12);
	set_int(0x2d, irq13);
	set_int(0x2e, irq14);
	set_int(0x2f, irq15);

	enable_int();
}

void set_int(unsigned char inte, void* funptr)
{
	unsigned long p = ((unsigned long)funptr & 0xffff0000);
	p >>= 16;
	unsigned long l = (((unsigned long)funptr & 0xffff) + (p << 48)
		+ 0x80000 + 0x8f0000000000);
	*(unsigned long*)(0x10 * (unsigned long)inte) = l;
}

#define PIC1_COMMAND			0x20
#define PIC1_DATA					0x21
#define PIC2_COMMAND			0xa0
#define PIC2_DATA					0xa1

#define ICW1_ICW4					0x01
#define ICW1_SINGLE				0x02
#define ICW1_INTERVAL4		0x04
#define ICW1_LEVEL				0x08
#define ICW1_INIT					0x10

#define ICW4_8086					0x01
#define ICW4_AUTO					0x02
#define ICW4_BUF_SLAVE		0x08
#define ICW4_BUF_MASTER		0x0c
#define ICW4_SFNM					0x10

// pretty much ripped from http://wiki.osdev.org/PIC
void pic_remap(int offset1, int offset2)
{
	unsigned char a1, a2;

	a1 = inb(PIC1_DATA);
	a2 = inb(PIC2_DATA);

	outb(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, (unsigned char)offset1);
	io_wait();
	outb(PIC2_DATA, (unsigned char)offset2);
	io_wait();
	outb(PIC1_DATA, 4);
	io_wait();
	outb(PIC2_DATA, 2);
	io_wait();

	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, a1);
	io_wait();
	outb(PIC2_DATA, a2);
	io_wait();
}

void irq_mask(unsigned char irq, unsigned char mask)
{
	unsigned char p, v;

	p = PIC1_DATA;

	if (irq >= 8) {
		p = PIC2_DATA;
		irq -= 8;
	}

	v = inb(p);

	if (mask == 0) {
		v &= ~(1 << irq);
	} else {
		v |= (1 << irq);
	}

	/*printk("set irq mask on ");
	printki(p);
	printk(" to ");
	printki(v);
	printk("\n");*/
	outb(p, v);
}


