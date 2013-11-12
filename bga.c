
extern unsigned short inw(unsigned short port);
extern void outw(unsigned short port, unsigned short val);

extern void printk(char* s);
extern void printki(long n);
extern void printkid(long n);

#define INDEX 	0x01ce
#define DATA  	0x01cf

#define ID      0
#define XRES    1
#define YRES    2
#define BPP     3
#define ENABLE  4
#define BANK    5
#define VWIDTH  6
#define VHEIGHT 7
#define XOFF 		8
#define YOFF 		9

// should probably read the PCI BAR rly, but oh well
#define LFB			0xe0000000

void bga_write(unsigned short index, unsigned short data)
{
	outw(INDEX, index);
	outw(DATA, data);
}

unsigned short bga_read(unsigned short index)
{
	outw(INDEX, index);
	return inw(DATA);
}

void setup_bga()
{
	printk("BGA ver ");
	printki(bga_read(ID));
	printk("\n");

	// set 1024x768x32
	printk("setting BGA settings\n");
	bga_write(ENABLE, 0);
	bga_write(XRES, 800);
	bga_write(YRES, 600);
	bga_write(BPP, 32);
	bga_write(ENABLE, 0x1 | 0x40); // enable extensions, enable LFB
	
	int x;
	int y;

	unsigned char* lfb = (unsigned char*)LFB;

	printk("drawing sample image to linear framebuffer\n");
	for (y = 0; y < 600; y++) {
		for (x = 0; x < 800; x++) {
			lfb[0] = (unsigned char)(y % 256);
			lfb[1] = (unsigned char)(y / 3);
			lfb[2] = (unsigned char)(x % 256);
			lfb[3] = 0;
			lfb += 4;
		}
	}

}


