

extern void printk(char* s);
extern void printki(long n);
extern void printkid(long n);

extern unsigned char inb(unsigned short port);
extern unsigned short inw(unsigned short port);
extern void outb(unsigned short port, unsigned char val);

extern void* kmalloc(long len);

#define BUS1 0x1f0
#define BUS2 0x170
#define BUS1CMD 0x3f6
#define BUS2CMD 0x376

void setup_ata_pio_bus(unsigned short iobase, unsigned short iocmd);

void read_sectors();

void setup_ata_pio()
{
	printk("------------------\n");
	printk("ATA PIO: begin setup\n");

	unsigned char status1;
	unsigned char status2;

	// read primary ATA status byte
	printk("ATA PIO: primary ATA bus status ");
	status1 = inb(BUS1 + 7);
	printki(status1);
	printk("\n");
	printk("ATA PIO: secondary ATA bus status ");
	status2 = inb(BUS2 + 7);
	printki(status2);
	printk("\n");

	if (status1 != 0xff) {
		setup_ata_pio_bus(BUS1, BUS1CMD);
	} else {
		printk("ATA PIO Bus 1 status was floating. Probably no drives.\n");
	}
	if (status2 != 0xff) {
		setup_ata_pio_bus(BUS2, BUS2CMD);
	} else {
		printk("ATA PIO Bus 2 status was floating. Probably no drives.\n");
	}

	read_sectors();

}

void setup_ata_pio_bus(unsigned short iobase, unsigned short iocmd)
{
	// do software reset of bus
	outb(iocmd, 4);
	inb(iobase + 7);
	inb(iobase + 7);
	inb(iobase + 7);
	inb(iobase + 7);
	outb(iocmd, 0);
	inb(iobase + 7);
	inb(iobase + 7);
	inb(iobase + 7);
	inb(iobase + 7);



	// get master status byte
	//
	// select master drive
	outb(iobase + 6, 0xa0);
	// burn some cycles
	inb(iobase + 7);
	inb(iobase + 7);
	inb(iobase + 7);
	inb(iobase + 7);

	unsigned char cl = inb(iobase + 4);
	unsigned char ch = inb(iobase + 5);

	printk("Master drive is ");
	if (cl == 0x14 && ch == 0xeb) {
		printk("PATAPI");
	} else if (cl == 0x69 && ch == 0x96) {
		printk("SATAPI");
	} else if (cl == 0x00 && ch == 0x00) {
		printk("PATA");
	} else if (cl == 0x3c && ch == 0xc3) {
		printk("SATA");
	} else {
		printk("?unknown?");
	}
	printk("\n");

	outb(iobase + 2, 0);
	outb(iobase + 3, 0);
	outb(iobase + 4, 0);
	outb(iobase + 5, 0);

	// send IDENTIFY cmd
	outb(iobase + 7, 0xec);

	unsigned char statusM = inb(iobase + 7);

	if (statusM != 0) {
		// poll status port until it 7 clears
		while ((inb(iobase + 7) & 0x80) > 0) {}

		// are lbamid and lbahi non-zero? if so, drive is not ata.
		if ((inb(iobase + 4) != 0) || (inb(iobase + 5) != 0)) {
			printk("Master drive not ATA; abandoning identify.\n");
		} else {
			// continue polling status until bit 3 set or bit 0 set
			unsigned char s;

			while (1) {
				s = inb(iobase + 7);
				if (((s & 0x8) > 0) || ((s & 0x1) != 0)) {
					break;
				}
			}

			if ((s & 0x1) != 0) {
				printk("Got ERR from identify; sorry.\n");
			} else {
				printk("Got DRQ set. Let's identify!\n");

				unsigned short idM[256];
				int a;

				for (a = 0; a < 256; a++) {
					idM[a] = inb(iobase);
				}

				if ((idM[0] & 0x8000) == 0) {
					printk("this is an ATA device\n");
				} else {
					printk("this is NOT an ATA device\n");
				}

				printk("firmware revision: ");
				char* u = "1";
				for (a = 23; a < 27; a++) {
					u[0] = idM[a];
					printk(u);
				}
				printk("\n");

				printk("model number: ");
				for (a = 27; a < 46; a++) {
					u[0] = idM[a];
					printk(u);
				}
				printk("\n");

				if ((idM[80] & 0x100) > 0) {
					printk("supports ATA8-ACS\n");
				}
				if ((idM[80] & 0x80) > 0) {
					printk("supports ATA/ATAPI-7\n");
				}
				if ((idM[80] & 0x40) > 0) {
					printk("supports ATA/ATAPI-6\n");
				}
				if ((idM[80] & 0x20) > 0) {
					printk("supports ATA/ATAPI-5\n");
				}
				if ((idM[80] & 0x10) > 0) {
					printk("supports ATA/ATAPI-4\n");
				}

				if ((idM[83] & 0x400) > 0) {
					printk("IDENTIFY says drive supports LBA48\n");
				}

				if ((idM[84] & 0x2) > 0) {
					printk("The SMART self-test is supported\n");
				}
				if ((idM[84] & 0x1) > 0) {
					printk("SMART error logging is supported\n");
				}

				printk("Ultra DMA modes supported: ");
				printki(idM[88] & 0xff);
				printk("\n");

				// master drive ONLY
				if ((idM[93] & 0x1000) > 0) {
					printk("drive detects an 80 pin cable\n");
				}

				//long lba48max = idM[100];
				printk("idM[100] ");
				printki(idM[100]);
				printk(" idM[101] ");
				printki(idM[101]);
				printk(" idM[102] ");
				printki(idM[102]);
				printk(" idM[103] ");
				printki(idM[103]);
				printk("\n");

				printk("idM[60] ");
				printki(idM[60]);
				printk(" idM[61] ");
				printki(idM[61]);
				printk("\n");

			}

		}
	} else {
		printk("Master drive does not exist on this bus.\n");
	}


	// do software reset of bus
	outb(iocmd, 4);
	inb(iobase + 7);
	inb(iobase + 7);
	inb(iobase + 7);
	inb(iobase + 7);
	outb(iocmd, 0);
	inb(iobase + 7);
	inb(iobase + 7);
	inb(iobase + 7);
	inb(iobase + 7);




	outb(iobase + 6, 0xb0);
	outb(iobase + 1, 0x00);
	outb(iobase + 1, 0x00);
	outb(iobase + 1, 0x00);
	outb(iobase + 1, 0x00);

	cl = inb(iobase + 4);
	ch = inb(iobase + 5);

	printk("Slave drive is ");
	if (cl == 0x14 && ch == 0xeb) {
		printk("PATAPI");
	} else if (cl == 0x69 && ch == 0x96) {
		printk("SATAPI");
	} else if (cl == 0x00 && ch == 0x00) {
		printk("PATA");
	} else if (cl == 0x3c && ch == 0xc3) {
		printk("SATA");
	} else {
		printk("?unknown?");
	}
	printk("\n");

	outb(iobase + 2, 0);
	outb(iobase + 3, 0);
	outb(iobase + 4, 0);
	outb(iobase + 5, 0);

	// send IDENTIFY cmd
	outb(iobase + 7, 0xec);

	unsigned char statusS = inb(iobase + 7);

	if (statusS != 0) {
		// poll status port until it 7 clears
		while ((inb(iobase + 7) & 0x80) > 0) {}

		// are lbamid and lbahi non-zero? if so, drive is not ata.
		if ((inb(iobase + 4) != 0) || (inb(iobase + 5) != 0)) {
			printk("Slave drive not ATA; abandoning identify.\n");
		} else {
			// continue polling status until bit 3 set or bit 0 set
			unsigned char t;

			while (1) {
				t = inb(iobase + 7);
				if (((t & 0x8) > 0) || ((t & 0x1) != 0)) {
					break;
				}
			}

			if ((t & 0x1) != 0) {
				printk("Got ERR from identify; sorry.\n");
			} else {
				printk("Got DRQ set. Let's identify!\n");

				unsigned short idS[256];
				int b;

				for (b = 0; b < 256; b++) {
					idS[b] = inb(iobase);
				}

				if ((idS[0] & 0x8000) == 0) {
					printk("this is an ATA device\n");
				} else {
					printk("this is NOT an ATA device\n");
				}

				printk("firmware revision: ");
				char* v = "1";
				for (b = 23; b < 27; b++) {
					v[0] = idS[b];
					printk(v);
				}
				printk("\n");

				printk("model number: ");
				for (b = 27; b < 46; b++) {
					v[0] = idS[b];
					printk(v);
				}
				printk("\n");

				if ((idS[80] & 0x100) > 0) {
					printk("supports ATA8-ACS\n");
				}
				if ((idS[80] & 0x80) > 0) {
					printk("supports ATA/ATAPI-7\n");
				}
				if ((idS[80] & 0x40) > 0) {
					printk("supports ATA/ATAPI-6\n");
				}
				if ((idS[80] & 0x20) > 0) {
					printk("supports ATA/ATAPI-5\n");
				}
				if ((idS[80] & 0x10) > 0) {
					printk("supports ATA/ATAPI-4\n");
				}

				if ((idS[83] & 0x400) > 0) {
					printk("IDENTIFY says drive supports LBA48\n");
				}

				if ((idS[84] & 0x2) > 0) {
					printk("The SMART self-test is supported\n");
				}
				if ((idS[84] & 0x1) > 0) {
					printk("SMART error logging is supported\n");
				}

				printk("Ultra DMA modes supported: ");
				printki(idS[88] & 0xff);
				printk("\n");

				// master drive ONLY
				if ((idS[93] & 0x1000) > 0) {
					printk("drive detects an 80 pin cable\n");
				}

				//long lba48max = idS[100];
				printk("idS[100] ");
				printki(idS[100]);
				printk(" idS[101] ");
				printki(idS[101]);
				printk(" idS[102] ");
				printki(idS[102]);
				printk(" idS[103] ");
				printki(idS[103]);
				printk("\n");


			}

		}
	} else {
		printk("Slave drive does not exist on this bus.\n");
	}

	outb(iobase + 6, 0x40);
	inb(iobase + 7);
	inb(iobase + 7);
	inb(iobase + 7);
	inb(iobase + 7);

}

void* disk1 = (void*) 0;
char d1sp[65536];

void make_sectors_available(unsigned long lba);

unsigned char rdiskb(unsigned long offset)
{
	make_sectors_available(offset / 512);
	return *((unsigned char*) (((unsigned long)disk1) + offset));
}

unsigned short rdiskw(unsigned long offset)
{
	make_sectors_available(offset / 512);
	return *((unsigned short*) (((unsigned long)disk1) + offset));
}

unsigned int rdiski(unsigned long offset)
{
	make_sectors_available(offset / 512);
	return *((unsigned int*) (((unsigned long)disk1) + offset));
}

unsigned long rdiskl(unsigned long offset)
{
	make_sectors_available(offset / 512);
	return *((unsigned long*) (((unsigned long)disk1) + offset));
}

void read_some_sectors(unsigned short iobase, unsigned long lba, unsigned short num_sectors);

void make_sectors_available(unsigned long lba)
{
	if ((d1sp[lba] == 0) || (d1sp[lba + 1] == 0)) {
		//printk("caching eight sectors\n");
		read_some_sectors(0x1f0, lba, 8);
	}
}

void read_sectors()
{
	int j;

	for (j = 0; j < 65536; j++) {
		d1sp[j] = 0;
	}

	disk1 = kmalloc(32 * 1024 * 1024);
}

void read_some_sectors(unsigned short iobase, unsigned long lba, unsigned short num_sectors)
{
	//unsigned long lba = 0;
	//unsigned short num_sectors = 0; // 65536 sectors == 32 MB

	outb(iobase + 2, (unsigned char)((num_sectors & 0xff00) >> 8));
	outb(iobase + 3, (unsigned char)((lba & 0xff000000) >> 24));
	outb(iobase + 4, (unsigned char)((lba & 0xff00000000) >> 32));
	outb(iobase + 5, (unsigned char)((lba & 0xff0000000000) >> 40));

	outb(iobase + 2, (unsigned char)(num_sectors & 0xff));
	outb(iobase + 3, (unsigned char)(lba & 0xff));
	outb(iobase + 4, (unsigned char)((lba & 0xff00) >> 8));
	outb(iobase + 5, (unsigned char)((lba & 0xff0000) >> 16));
	
	outb(iobase + 7, 0x24);

	unsigned short* d = (unsigned short*)((unsigned long)disk1 + lba * 512);
	unsigned char e = 0;

	unsigned long j;
	for (j = lba; j < lba + num_sectors; j++) {
		d1sp[j] = 1;
	}

	for (; ((unsigned long)d) < (((unsigned long)disk1) + lba * 512 + num_sectors * 512); ) {
		unsigned char status = inb(iobase + 7);

		while (1) {
			status = inb(iobase + 7);

			// BSY clear && DRQ set?
			if ( ((status & 0x80) == 0) && ((status & 0x8) > 0) ) {
				break;
			}

			// ERR set || DF set?
			if ( ((status & 0x1) > 0) || ((status & 0x20) > 0) ) {
				e = 1;
				break;
			}
		}

		if (e == 1) {
			printk("Error reading disk (ptr ");
			printki((long)d);
			printk(")\n");
			break;
		}
		
		int i;
		for (i = 0; i < 256; i++) {
			*d = inw(iobase);
			d++;
		}

		inb(iobase + 7);
		inb(iobase + 7);
		inb(iobase + 7);
		inb(iobase + 7);
	}

	//printk("Disk read finished\n");

	
}

