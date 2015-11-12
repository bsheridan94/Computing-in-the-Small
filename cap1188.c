#include <u.h>
#include <libc.h>

#define LEDLINK 0x72
#define PRODID 0xFD
#define MANUID 0xFE
#define REV 0xFF
#define LEDPOL 0x73
#define MTBLK 0x2A
#define BUTTONSTATUS 0x3
#define WRITELED 0x74
#define MAINREG 0x00
#define MAINREG_INT 0x01

void
main()
{
	int fd;
	uchar buf[256];
	uchar value[1];

 	// Base i2c address is 0x29
	fd = open("/dev/i2c.29.data", ORDWR);
	if(fd< 0){
		bind("#J29","/dev", MAFTER);
		fd = open("/dev/i2c.29.data", ORDWR);
		if(fd < 0)
			print("open error: %r\n");
	}

	//Read in inital register states
	// Writes address of read. 0x00
	value[0] = 0x00;
	pwrite(fd, value, 1, 0);
	// Reads in contents of all registers
	pread(fd, buf, 256, 0);


	// Checks that register read was sucessful through 
	// checking product id, manuf id and rev
	if(buf[PRODID] != 0x50)
		print("Product ID: 0x%x\n",buf[PRODID]);
	if(buf[MANUID] != 0x5D)
		print("Product Manuf ID: 0x%x\n",buf[MANUID]);
	if(buf[REV] != 0x83)
		print("Product REV: 0x%x\n",buf[REV]);

	// Enables LED linking to Input
	buf[LEDLINK] = 0xff;
	// Speeds up sampling 
	buf[0x41] = 0x30; 
	//writes setup configuration to sensor
	pwrite(fd, buf, 256, 0);
		
	// Reads sensor input until sensor 8 is touched
	while(buf[BUTTONSTATUS] != 0x80){

		//Clears the intterupt then reads in the registers
		buf[MAINREG] = buf[MAINREG] & ~MAINREG_INT;
		pwrite(fd, buf, 256, 0);
		
		// reads in all registers
		pwrite(fd, value, 1, 0);
		pread(fd, buf, 256, 0);

		int i=0;
		// Reads in button status and blinks like a counter.
		// will take a few seconds to run. does not change
		// till the counting has ended
		if(buf[BUTTONSTATUS] == 0x01){
			for(i=0;i<256;i++){
				buf[WRITELED] = i;
				// writes to led control register
				pwrite(fd, buf, 256, 0);
				sleep(100);
			}
		}

		// Reads in button status and blinks all odd
		// then even leds.
		if(buf[BUTTONSTATUS] == 0x02){
				buf[WRITELED] = 0xAA;
				pwrite(fd, buf, 256, 0);
				sleep(1000);
				buf[WRITELED] = 0x55;
				pwrite(fd, buf, 256, 0);
				sleep(1000);
		}

		// Mode 3 All LED's On
		if(buf[BUTTONSTATUS] == 0x04){
			buf[WRITELED] = 0xFF;
			pwrite(fd, buf, 256, 0);
		}	
			
		// Mode 4 All LED's OFF
		if(buf[BUTTONSTATUS] == 0x08){
			buf[WRITELED] = 0x00;
			pwrite(fd, buf, 256, 0);
		}

	// Prints out button status to screen	
	print("Button Status: %x\n", buf[BUTTONSTATUS]);
	sleep(1000);
	}


	


}	
