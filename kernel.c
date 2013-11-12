void printki(long num);
void printkid(long num);
void printk(char* s);

extern void outb(unsigned short port, unsigned char val);
extern unsigned char inb(unsigned short port);

extern void pci_thing();

extern void some_ps2_stuff();

extern void some_serial_stuff();

extern void setup_apic();
extern void setup_apic_notreally();

extern void setup_ata_pio();

extern void setup_bga();

// imagine dragons - radioactive

void* kmstart;
void* kmlimit;

void _start()
{
  int i; int j;

  printk("I apparently have ");
  printkid(*((short*)0x5020));
  printk(" MB of rams\n");
  printk("sizeof int (should be 4): ");
  printkid(sizeof(int));
  printk(" sizeof long (should be 8): ");
  printkid(sizeof(long));
  printk("\n");

  printk("E820 System Memory Map:\n");

  long starts[16];
  long lengths[16];
  int goodCount = 0;

  long* wat = (long*)0x4000;
  for (i = 0; i < 16; i++) {
    /*long* startAddr = (long*)((long)0x4000 + 24 * i);
    long* length = (long*)((long)0x4000 + 24 * i + 1);
    int* memType = (int*)((long)0x4000 + 24 * i + 16);
    int* acpi = (int*)((long)0x4000 + 24 * i + 20);
    printk(" start ");
    printki(*startAddr);
    printk(" len ");
    printki(*length);
    printk(" memType ");
    printkid(*memType);
    printk(" acpi ");
    printkid(*acpi);
    printk("\n");*/
    long startAddr = *wat;
    wat++;
    long length = *wat;
    wat++;
    int type = (int)(*wat >> 32);
    int acpi = (int)(*wat);
    wat++;

    // usable high memory?
    if (type == 1 && startAddr >= 0x100000) {

      printki(startAddr);
      printk(" ");
      printki(length);
      printk(" ");
      printkid(type);
      printk(" ");
      printkid(acpi);
      printk("\n");

      starts[goodCount] = startAddr;
      lengths[goodCount] = length;
      goodCount++;

      if (goodCount == 16)
        break;

    }

    /*for (j = 0; j < 2; j++) {
      printki(*wat);
      printk(" ");
      wat++;
    }
    j = (int)(*wat >> 32);
    printkid(j);
    printk(" ");
    j = (int)(*wat);
    printkid(j);
    printk("\n");*/
  }

  int biggest = 0;

  if (goodCount == 0) {
    printk("I didn't find any suitable high memory. Email leaf@leaf.sx!\n");
    while(1) {}
  } else {
    printk("I found at least one suitable high memory area.\n");
    // biggest mem area?
    for (j = 0; j < goodCount; j++) {
      if (lengths[j] > biggest)
        biggest = j;
    }
    printk("Biggest contiguous memory area is ");
    printkid(lengths[biggest] / (1024*1024));
    printk(" MiB.\n");

    if (lengths[biggest] < (256*1024*1024)) {
      printk("THIS IS SMALLER THAN 256 MiB!  OS may fail. Let's try, though.\n");
    }
  }

  kmstart = (void*) starts[j];
  kmlimit = (void*) (starts[j] + lengths[biggest]);

  pci_thing();

	//setup_apic();
	setup_apic_notreally();
	setup_pic();

	some_ps2_stuff();

	some_serial_stuff();

	setup_ata_pio();

	setup_bga();

  while (1) { }

}

// Simple memory allocation.
// 'Real' memory allocation is performed after setup is done by taking
//  control of a huge chunk of memory, and another malloc func does caretaking.
void* kmalloc(long bytes) {
  void* m = kmstart;
  kmstart += bytes;
  if (kmstart >= kmlimit) {
    printk("OUT OF MEMORY!\n");
    while(1) {}
  }
  return m;
}

// backbuffer for vid screen
char vidfriendback[2000];
char* vidfriendfront = (char*)0xb8000;
int vidfriendpos = 0;

void printkid(long num) {
  char kibi[64];
  int i;
  long num2;

  for (i = 0; i < 64; i++) {
    kibi[i] = 0;
  }
  kibi[0] = '0';

  num2 = num;
  i = -1;
  for (; num2 > 0; num2 /= 10) {
    i++;
  }

  for (; num > 0; num /= 10) {
    kibi[i] = (num % 10) + '0';
    i--;
  }

  printk(kibi);
}

void printki(long num) {
  char kibi[64];
  int i;
  long num2;

  for (i = 0; i < 64; i++) {
    kibi[i] = 0;
  }
  kibi[0] = '0';
  kibi[1] = 'x';
  kibi[2] = '0';

  num2 = num;
  i = 1;
  for (; num2 > 0; num2 /= 16) {
    i++;
  }

  for (; num > 0; num /= 16) {
    if (num % 16 > 9) {
      kibi[i] = ((num % 16) - 10) + 'a';
    } else {
      kibi[i] = (num % 16) + '0';
    }
    i--;
  }

  printk(kibi);
}

void printk(char* s) {
	char* t = s;
  char* vf = (char*)(vidfriendfront + 2 * vidfriendpos);
  char* vfb = (char*)(vidfriendback + vidfriendpos);
  int i;

  for (; *s != 0; s++) {
    if (*s == '\n') {
      // insert spaces
      for (; (vidfriendpos % 80) != 0; ) {
        *vf = ' ';
        *vfb = ' ';
        vf++; vfb++;
        *vf = 7;
        vf++;
        vidfriendpos++;
      }
      continue;
    }

    if (vidfriendpos >= 25*80) {
      for (i = 0; i < 24*80; i++) {
        vidfriendback[i] = vidfriendback[i + 80];
      }
      for (i = 24*80; i < 25*80; i++) {
        vidfriendback[i] = ' ';
      }
      // now blit backbuffer to front
      vf = (char*)vidfriendfront;
      vfb = (char*)vidfriendback;
      for (i = 0; i < 25*80; i++) {
        *vf = *vfb;
        vf++;
        *vf = 7;
        vf++;
        vfb++;
      }
      // fix pos
      vidfriendpos = 24*80;
      vf = (char*)(vidfriendfront + 2 * vidfriendpos);
      vfb = (char*)(vidfriendback + vidfriendpos);
    }

    *vf = *s;
    vf++;
    *vf = 7;
    vf++;
    *vfb = *s;
    vfb++;
    vidfriendpos++;
  }

	// give to serial, but don't do it for message 'COM1 IRQ'
	if (t[0] != 'C' || t[3] != '1' || t[5] != 'I') {
		serial_printk(t);
	}
}

