extern void outb(unsigned short port, unsigned char val);
extern unsigned char inb(unsigned short port);
extern void outd(unsigned short port, unsigned int val);
extern unsigned int ind(unsigned short port);
extern void outw(unsigned short port, unsigned short val);
extern unsigned short inw(unsigned short port);

extern void printki(long n);
extern void printk(char* s);
extern unsigned long simple();
extern unsigned long first(unsigned long a, unsigned long b);
extern unsigned long snd(unsigned long a, unsigned long b);

unsigned int pcireadw(unsigned short bus, unsigned short slot,
                        unsigned short fun, unsigned short offy)
{
  unsigned int address;
  
  address = 0x80000000 | ((unsigned int)bus) << 16 | ((unsigned int)slot) << 11
    | ((unsigned int)fun) << 8 | offy;

  outd(0xCF8, address);
  return ind(0xCFC);
}

void check_fun(unsigned short bus, unsigned short slot, unsigned short fun)
{
  unsigned int devven = pcireadw(bus, slot, fun, 0);
  
  if (devven == 0xFFFFFFFF)
    return;

  unsigned int stacmd = pcireadw(bus, slot, fun, 0x4);
  unsigned int classes = pcireadw(bus, slot, fun, 0x8);
  unsigned int bhlc = pcireadw(bus, slot, fun, 0xc);

  int dev = (devven >> 16);
  int ven = devven & 0xffff;
  //int status = (stacmd >> 16); // -Wunused-variable
  //int cmd = stacmd & 0xffff; // -Wunused-variable
  int class = classes >> 24;
  int subclass = (classes >> 16) & 0xff;
  int progif = (classes >> 8) & 0xff;
  int header = (bhlc >> 16) & 0xff;
  int multifunction = ((header & 0x80) == 0x80);

  printk("h ");
  printki(header);
  printk(" classes ");
  printki(classes);

  int blame = 0;

  switch (class) {
    case 0x00:
      if (progif == 0x00) {
        switch (subclass) {
          case 0x00:
            printk("Any device except for VGA-Compatible devices");
            break;
          case 0x01:
            printk("VGA-Compatible device");
            break;
          default:
            printk(" class 0x00 subclass ");
            printki(subclass);
            printk(" progif ");
            printki(progif);
        }
      } else {
        blame = 1;
      }
      break;

    case 0x1:
      if (subclass == 0x01) {
        printk("c0x01 sc0x01 pif0x?? IDE Controller");
      } else if (subclass == 0x06) {
        if (progif == 0x00) {
          printk("c0x01 sc0x06 pif0x00 Serial ATA (Vendor Specific Interface)");
        } else if (progif == 0x01) {
          printk("c0x01 sc0x06 pif0x01 Serial ATA (AHCI 1.0)");
        } else {
          blame = 1;
        }
      } else {
        blame = 1;
      }
      break;

    case 0x2:
      if (subclass == 0x00 && progif == 0x00) {
        printk("c0x02 sc0x00 pif0x00 Ethernet Controller");
        printk(" v");
        printki(ven);
        printk(" d");
        printki(dev);
      } else {
        blame = 1;
      }
      break;

    case 0x3:
      if (subclass == 0x00 && progif == 0x00) {
        printk("c0x03 sc0x00 pif0x00 VGA-Compatible Controller");
				printk(" v");
				printki(ven);
				printk(" d");
				printki(dev);
      } else {
        blame = 1;
      }
      break;

    case 0x6:
      if (subclass == 0x00 && progif == 0x00) {
        printk("c0x06 sc0x00 pif0x00 Host bridge");
      } else if (subclass == 0x01 && progif == 0x00) {
        printk("c0x06 sc0x01 pif0x00 ISA bridge");
      } else if (subclass == 0x80 && progif == 0x00) {
        printk("c0x06 sc0x80 pif0x00 Other bridge device");
      } else {
        blame = 1;
      }
      break;

    case 0x8:
      if (subclass == 0x80 && progif == 0x00) {
        printk("c0x08 sc0x80 pif0x00 Other system peripheral");
      } else {
        blame = 1;
      }
      break;

    case 0xc:
      if (subclass == 0x03 && progif == 0x00) {
        printk("c0x0c sc0x03 pif0x00 USB UHCI");
      } else if (subclass == 0x03 && progif == 0x10) {
        printk("c0x0c sc0x03 pif0x10 USB OHCI");
      } else if (subclass == 0x03 && progif == 0x20) {
        printk("c0x0c sc0x03 pif0x20 USB2 EHCI");
      } else {
        blame = 1;
      }
      break;

    default:
      blame = 1;
      break;
  }

  if (blame) {
    printk(" class ");
    printki(class);
    printk(" subclass ");
    printki(subclass);
    printk(" progif ");
    printki(progif);
    printk(" :: Unrecognized device");
  }

  if (multifunction) {
    printk(" (MULTIFUNCTION)");
  }

  printk("\n");

  //unsigned int bhlc = pcireadw(bus, slot, fun, 0xc);

	int z;
	unsigned short y;
	int a = 0;
	for (z = 0; z < 6; z++) {
		y = pcireadw(bus, slot, fun, 0x10 + 0x4 * z);
		if (y != 0) {
			if (a > 0) {
				printk(" BAR");
			} else {
				printk("BAR");
			}
			printkid(z);
			printk(" ");
			printki(y);
			a++;
		}
	}
	if (a > 0) {
		printk("\n");
	}

  if (multifunction && fun == 0x00) {
    unsigned short f = 1;
    for (; f < 8; f++) {
      check_fun(bus, slot, f);
    }
  }
}

void pci_thing()
{
  printk("test:\n");
  printki(simple());
  printk(" ");
  printki(first(0x1234, 0x5678));
  printk(" ");
  printki(snd(0x1234, 0x5678));
  printk("\nResults from configreading bus 0 slot 0:\n");
  unsigned short device;
  for (device = 0; device < 32; device++)
  {
    check_fun(0, device, 0);
  }
}
