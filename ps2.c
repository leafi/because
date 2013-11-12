extern unsigned char inb(unsigned short port);
extern unsigned int ind(unsigned short port);
extern unsigned short inw(unsigned short port);

extern void outb(unsigned short port, unsigned char val);
extern void outd(unsigned short port, unsigned int val);
extern void outw(unsigned short port, unsigned short val);

extern void printki(long n);
extern void printk(char* s);

extern void set_int(unsigned char inte, void* funptr);
extern void disable_int();
extern void enable_int();
extern void irq_mask(unsigned char irq, unsigned char mask);

extern void irq1_ps2_port1();

void wait_for_in_clear()
{
	while ((inb(0x64) & 0x2) > 0) {}
}

void wait_for_out_data()
{
	while ((inb(0x64) & 0x1) == 0) {}
}

void some_ps2_stuff()
{
	// TODO: init usb (disable usb-ps/2 legacy support)

	// TODO: ensure there actually is a PS/2 controller (acpi?)

	disable_int();

	// disable ps/2 ports
	printk("disable ps/2 ports...\n");
	// - send 0xad
	outb(0x64, 0xad);
	// - send 0xa7
	outb(0x64, 0xa7);

	// flush the output buffer
	// - read a bunch
	int i = 0;
	while ((inb(0x64) & 1) == 1)
	{
		inb(0x60);
	}

	printk("flushed ");
	printki(i);
	printk(" bytes from ps/2 buffer\n");


	// set CCB
	printk("set ccb\n");
	// (disable all IRQs & disable translation (clear bits 0, 1 and 6))
	outb(0x64, 0x20);
	unsigned char ccb = inb(0x60);
	// disable irqs
	ccb &= 0xfc;
	// disable PS/2 port 1 translation (we don't do mode 1 keybs anyway)
	ccb &= 0xbf;
	// actually set
	outb(0x64, 0x60);
	outb(0x60, ccb);

	char singlechannel = 0;
	if ((ccb & 0x20) > 0) {
		printk("might be a two-port ps/2 controller\n");
	} else {
		printk("can't be a two-port ps/2 controller\n");
		singlechannel = 1;
	}



	// perform controller self test
	// - send 0xaa
	// - ensure response is 0x55
	printk("self test result: ");
	outb(0x64, 0xaa);
	unsigned char response = inb(0x60);
	if (response == 0x55) {
		printk("OK\n");
	} else if (response == 0xfc) {
		printk("Fail\n");
		// TODO: do something here
	} else {
		printk("???\n");
		// TODO: do something here
	}



	// determine if there are 2 channels
	if (!singlechannel) {
		printk("second-chance test if this ps/2 controller has only 1 port\n");
		// enable second ps/2 port & test if bit 5 of ccb is clear
		outb(0x64, 0xa8);

		outb(0x64, 0x20);
		char ccb2 = inb(0x60);

		if ((ccb2 & 0x20) == 0) {
			printk("this is actually a single channel controller (broken test; ignoring)\n");
			//singlechannel = 1;
		}
	}


	// perform controller self-test
	outb(0x64, 0xaa);
	printk("ps/2 controller self test: ");
	if (inb(0x60) == 0x55) {
		printk("OK\n");
	} else {
		printk("fail\n");
	}


	// perform interface tests
	// - send 0xab, check result
	printk("ps/2 controller port 1 test: ");
	outb(0x64, 0xab);
	char port1result = inb(0x60);
	if (port1result != 0x00) {
		printki(port1result);
		printk("\n");
	} else {
		printk("OK\n");
	}

	// - send 0xa9, check result
	char port2result = 0;
	if (!singlechannel) {
		printk("ps/2 controller port 2 test: ");
		outb(0x64, 0xa9);
		port2result = inb(0x60);
		if (port2result != 0x00) {
			printki(port2result);
			printk("\n");
		} else {
			printk("OK\n");
		}
	}



	// enable ports again
	// - enable port 1
	if (port1result == 0) {
		printk("enable ps/2 port 1\n");
		outb(0x64, 0xae);
	}
	// - enable port 2
	if (!singlechannel && (port2result == 0)) {
		printk("enable ps/2 port 2\n");
		outb(0x64, 0xa8);
	}
	// - enable interrupts by modding ccb
	printk("enabling interrupts on ps/2\n");
	outb(0x64, 0x20);
	unsigned char ccb2 = inb(0x60);

	printk("ccb was ");
	printki(ccb2);
	printk("\n");
	if (port1result == 0) {
		ccb2 |= 0x1;
	}
	if (!singlechannel && (port2result == 0)) {
		ccb2 |= 0x2;
	}
	printk("ccb is now going to be ");
	printki(ccb2);
	printk("\n");

	outb(0x64, 0x60);
	outb(0x60, ccb2);



	// reset devices:
	// - ensure 'input buffer' clear
	if (port1result == 0) {
		printk("wait for ps/2 'input buffer' clear\n");
		int z = 0;
		while ((inb(0x64) & 0x2) > 0) {
			// ...
			z++;
		}
		printki(z);
		printk(" time(s) to clear\n");
		printk("send reset to ps/2 port 1\n");
		outb(0x60, 0xff); // send reset cmd to kbd
		printk("wait for ps/2 output buffer mark\n");
		while ((inb(0x64) & 0x1) == 0) {
			// ...
		}

		unsigned char resetres1;
		
		while (1) {
			resetres1 = inb(0x60);
			printk("r&sst result: ");

			if (resetres1 == 0xfa) {
				if ((inb(0x64) & 0x1) == 1) {
					resetres1 = inb(0x60);
				}
			}

			if (resetres1 == 0xaa) {
				printk("OK");
				break;
			} else if ((resetres1 == 0xfc) || (resetres1 == 0xfd)) {
				printk("self-test failed");
				break;
			} else if (resetres1 == 0xfe) {
				printk("(resend)");
				// TODO: resend
			} else {
				printk("?? ");
				printki(resetres1);
				break;
			}
		}
		printk("\n");

		// detect ps/2 port 1 device
		// - disable scanning
		while (1) {
			wait_for_in_clear();
			outb(0x60, 0xf5);
			wait_for_out_data();
			if (inb(0x60) == 0xfa) {
				break;
			}
		}

		// - send identify 0xf2
		while (1) {
			wait_for_in_clear();
			outb(0x60, 0xf2);
			wait_for_out_data();
			char rlybrk;
			if (inb(0x60) == 0xfa) {
				// todo: consider 0 byte case? meh...
				wait_for_out_data();
				unsigned char ps2id1;
				unsigned char ps2id12;
				ps2id1 = inb(0x60);
				rlybrk = 1;
				switch (ps2id1) {
					case 0x00:
						printk("ps/2 port 1: standard ps/2 mouse\n");
						break;
					case 0x03:
						printk("ps/2 port 1: mouse with scroll wheel\n");
						break;
					case 0x04:
						printk("ps/2 port 1: 5-button mouse\n");
						break;
					case 0xab:
						ps2id12 = inb(0x60);
						if ((ps2id12 == 0x41) || (ps2id12 == 0xc1)) {
							printk("ps/2 port 1: mf2 kbd with translation enabled\n");
						} else if (ps2id12 == 0x83) {
							printk("ps/2 port 1: mf2 kbd\n");
						} else {
							rlybrk = 0;
						}
						break;
					default:
						rlybrk = 0;
						break;
				}
			}
			if (rlybrk == 1) {
				break;
			}

		}

		// enable scanning
		outb(0x60, 0xf4);
		wait_for_out_data();
		inb(0x60);

		// just get keys. temp...
		/*printk("\nsay wait keys\n");
		wait_for_in_clear();
		outb(0x60, 0xf4);
		//outb(0x60, 0xee);
		//printk("ok... let's go!\n");
		wait_for_out_data();
		inb(0x60);*/
		/*while (1) {
			wait_for_out_data();
			printki(inb(0x60));
			printk(" ");
		}*/

		set_int(0x21, &irq1_ps2_port1);
		irq_mask(1, 0);
	}

	// TODO; reset port 2 device.
	if (!singlechannel && (port2result == 0)) {
		printk("TODO: reset port 2 device\n");
	}

	printk("\nps/2 controller status reg: ");
	printki(inb(0x64));
	printk("\n");

	enable_int();
}

