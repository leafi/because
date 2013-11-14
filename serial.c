extern unsigned char inb(unsigned short port);
extern void outb(unsigned short port, unsigned char val);

extern void printki(long n);
extern void printk(char* s);

extern void disable_int();
extern void enable_int();

extern void set_int(unsigned char inte, void* funptr);
extern void irq_mask(unsigned char irq, unsigned char mask);

extern void a_irq4_handler();

extern void testq();

char serial_started = 0;

#define PORT 0x3f8

#define QUEUE_LENGTH 65536

char send_queue[QUEUE_LENGTH];
//char[QUEUE_LENGTH] recv_queue;
int send_start = 0;
int send_end = 0;
//int recv_start = 0;
//int recv_end = 0;

void serial_poll();

void irq4_handler()
{
	printk("COM1 IRQ\n");
	serial_poll();
}

void serial_poll()
{
	if (!serial_started) {
		//printk("COM1 not started yet. Cannot continue.\n");
		return;
	}

	// empty input buffer com1
	while ((inb(PORT + 5) & 1)) {
		printki(inb(PORT));
	}

	// send output buffer com1
	while (send_start != send_end) {
		while ((inb(PORT + 5) & 0x20) == 0) {}
		outb(PORT, send_queue[send_start]);
		send_start = (send_start + 1) % QUEUE_LENGTH;
	}

}

void serial_printk(char* s)
{
	for (; *s != 0; s++) {
		send_queue[send_end] = *s;
		send_end = (send_end + 1) % QUEUE_LENGTH;
	}

	serial_poll();
}

void some_serial_stuff()
{
	disable_int();

	printk("COM1: setting up\n");

	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x80);
	outb(PORT + 0, 0x03);
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x03);
	outb(PORT + 2, 0xc7);
	//outb(PORT + 2, 0x07); // debugging: trigger every byte
	outb(PORT + 4, 0x0b);

	set_int(0x24, &a_irq4_handler);
	irq_mask(4, 0);

	enable_int();

	serial_started = 1;

	printk("COM1: should be upset now\n");

	serial_poll();
}
