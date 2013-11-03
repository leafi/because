void printki(long num);
void printkid(long num);
void printk(char* s);

extern void outb(unsigned short port, unsigned char val);
extern unsigned char inb(unsigned short port);

extern void pci_thing();

/*inline void outb(unsigned short port, unsigned char val)
{
  //asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
  asm volatile("outb %%al,%%dx": :"d" (port), "a"(val));
}*/
/*static inline void outw(unsigned short port, unsigned short val)
{
  asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}
static inline void outd(unsigned short port, unsigned int val)
{
  asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}
static inline unsigned char inb(unsigned short port)
{
  unsigned char ret;
  //asm volatile("in %1, %0" : "=a"(ret) : "Nd"(port) );
  asm volatile("inb %%dx,%%al":"=a" (ret):"d" (port));
  return ret;
}
static inline unsigned short inw(unsigned short port)
{
  unsigned short ret;
  asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}
static inline unsigned int ind(unsigned short port)
{
  unsigned int ret;
  asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}*/

unsigned char dontcare = 0;

/*static void w3C0(unsigned char index, unsigned char data)
{
  dontcare += inb(0x3DA); // go to index state
  outb(0x3C0, index);
  outb(0x3C0, data);
}

static unsigned char r3C0(unsigned char index)
{
  dontcare += inb(0x3DA); // go to index state
  outb(0x3C0, index);
  return inb(0x3C1);
  // if not doing inb(0x3DA) before 3C0 accesses do read 0x3DA here!
}

static unsigned char r3C2()
{
  return inb(0x3CC);
}

static void w3C2(unsigned char data)
{
  outb(0x3C2, data);
}

static unsigned char r3C4(unsigned char index)
{
  outb(0x3C4, index);
  return inb(0x3C5);
}

static void w3C4(unsigned char index, unsigned char data)
{
  outb(0x3C4, index);
  outb(0x3C5, data);
}

static unsigned char r3CE(unsigned char index)
{
  outb(0x3CE, index);
  return inb(0x3CF);
}

static void w3CE(unsigned char index, unsigned char data)
{
  outb(0x3CE, index);
  outb(0x3CF, data);
}

static unsigned char r3D4(unsigned char index)
{
  outb(0x3D4, index);
  return inb(0x3D5);
}

static void w3D4(unsigned char index, unsigned char data)
{
  outb(0x3D4, index);
  outb(0x3D5, data);
}*/

// imagine dragons - radioactive

void* kmstart;
void* kmlimit;

void _start()
{
  int i; int j;

  printk("I apparently have ");
  printkid(*((short*)0x5020));
  printk(" MB of rams\n");
  printk("sizeof int : ");
  printkid(sizeof(int));
  printk(" sizeof long : ");
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


  /*int yy = -1;
  int z;
  for (z = 0; z < 10000; z++) {
    yy *= z;
  }*/

  //asm("cli");

  /*unsigned char good = 0x02;
  while (good & 0x02)
    good = inb(0x64);
  outb(0x64, 0xFE);
  asm("hlt");*/

  //outb(0x70,0x30);

  

  // start vga (mode 12h; 640x480 planar 16-bit)
  /*w3C0(0x10, 0x01);
  w3C0(0x11, 0x00);
  w3C0(0x12, 0x0F);
  w3C0(0x13, 0x00);
  w3C0(0x14, 0x00);
  w3C2(0xE3);
  printki(r3C2(0xE3));
  w3C4(0x01, 0x01);
  w3C4(0x03, 0x00);
  w3C4(0x04, 0x02);
  w3CE(0x05, 0x00);
  w3CE(0x06, 0x05);
  w3D4(0x00, 0x5F);
  w3D4(0x01, 0x4F);
  w3D4(0x02, 0x50);
  w3D4(0x03, 0x82);
  w3D4(0x04, 0x54);
  w3D4(0x05, 0x80);
  w3D4(0x06, 0x0B);
  w3D4(0x07, 0x3F);
  w3D4(0x08, 0x00);
  w3D4(0x09, 0x40);
  w3D4(0x10, 0xEA);
  w3D4(0x11, 0x8C);
  w3D4(0x12, 0xDF);
  w3D4(0x13, 0x28);
  w3D4(0x14, 0x00);
  w3D4(0x15, 0xE7);
  w3D4(0x16, 0x04);
  w3D4(0x17, 0xE3);*/
  

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
}

