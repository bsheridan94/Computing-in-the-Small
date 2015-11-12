#include <u.h>
#include <libc.h>

#define SOFTWARE_REST 0x01
#define SLEEP_OUT 0x11
#define PARTIAL_OFF 0x13
#define DISPLAY_ON 0x29

void
main()
{

	/*
	Wiring

	Lite -> 3.3 V
	Miso -> N/A
	SCK -> SCLK
	MOSI -> MOSI
	TFT_CS -> Pin 23
	Card_CS -> N/A
	D/C -> Pin 25
	Reset -> pin 24
	VCC -> 3.3 V
	GND -> GND

	*/

	int spifd;
	int gpio;
	int i = 0;
	int j = 0;

	// Command array
	uchar command[32];
	//Stores data to write to screen
	int screenBuf[160][128][2];

	// Initializes screen to black
    for(i=0;i<160;i++)
		for(j=0;j<128;j++){
			screenBuf[i][j][0] = 0x00; 
			screenBuf[i][j][1] = 0x00; 
		}

	// Initialize SPI Device
 	spifd = open("/dev/spi0",ORDWR);
	if(spifd < 0){
		bind("#π","/dev",MAFTER);
		spifd = open("/dev/spi0",ORDWR);
		if(spifd < 0){
			print("error with SPI");
		}
	}

	//binds the gpio
	bind("#G", "/dev", MAFTER);
	gpio = open("/dev/gpio", ORDWR);
	

	//Set D/C
	// 1 data
	// 0 command
	fprint(gpio, "function 25 out");
	fprint(gpio, "set 25 0");

	//Set Reset 
	fprint(gpio, "function 24 out");
	fprint(gpio, "set 24 1");

	//Set CS
	fprint(gpio, "function 23 out");
	fprint(gpio, "set 23 0");



	// Set Software Reset
	command[0] = 0x01;
	pwrite(spifd, &command, 1, 0);
	sleep(500);

	// Set Sleep out
	command[0] = 0x11;
	pwrite(spifd, &command, 1, 0);
	sleep(500); 

	// Set normal display mode
	command[0] = 0x13;
	pwrite(spifd, &command, 1, 0);
	sleep(500); 

	// Set display on
	command[0] = 0x29;
	pwrite(spifd, &command, 1, 0);
	sleep(500);

	// Set color mode to 16 bit 5-4-5
	command[0] = 0x3a;
	pwrite(spifd, &command, 1, 0);
	sleep(500);

	fprint(gpio, "set 25 1");
	command[0] = 0x05;
	pwrite(spifd, &command, 1, 0);
	sleep(500);
	fprint(gpio, "set 25 0");
	

	// Set Cols 
	command[0] = 0x2a;
	pwrite(spifd, &command, 1, 0);
	sleep(500);

	fprint(gpio, "set 25 1");
	command[0] = 0x00;
	command[1] = 0x00;
	command[2] = 0x00;
	command[3] = 127;
	pwrite(spifd, &command, 4, 0);
	sleep(500);
	fprint(gpio, "set 25 0");

	//Set Rows 
	command[0] = 0x2b;
	pwrite(spifd, &command, 1, 0);
	sleep(500);

	fprint(gpio, "set 25 1");
	command[0] = 0x00;
	command[1] = 0x01;
	command[2] = 0x00;
	command[3] = 159;
	pwrite(spifd, &command, 4, 0);
	sleep(500);
	fprint(gpio, "set 25 0");

	// Call RamWrite
	command[0] = 0x2c;
	pwrite(spifd, &command, 1, 0);
	sleep(500);
	
	fprint(gpio, "set 25 1");
	//Mashes all memory to be 0, which should be white

	//Write current ScreenBuf to memory
	for(i=0;i<160;i++)
		for(j=0;j<128;j++){
			command[0] = screenBuf[i][j][0]; 
			command[1] = screenBuf[i][j][1];
			pwrite(spifd, &command, 2, 0);
		}

	fprint(gpio, "set 25 0");
	sleep(1000);


	/*
		The code writes 4 giants stripes to the screen. Each Stripe is a different color. 
		Each second the screen is rewritten, and each Stripe is cycled down one. THe bottom is then moved back on 
		to the top. The code runs forever generating the animation.

		The use of the screenbuf allows the program to draw what ever it would like through accessing the array and writing the RGB 5-6-5 code to it.
		This stripe animation proves the program has full control of the screen.

	*/

		while(1){

		for(i=0;i<160;i++)
			for(j=0;j<128;j++){

				if(i >= 0 && i < 40){
					// Red 
					screenBuf[i][j][0] = 0xF8; 
					screenBuf[i][j][1] = 0x00; 
				}

				if(i >= 40 && i < 80){
					// Blue
					screenBuf[i][j][0] = 0x00; 
					screenBuf[i][j][1] = 0x1F; 
				}

				if(i >= 80 && i < 120){
					// Orange 
					screenBuf[i][j][0] = 0xFC; 
					screenBuf[i][j][1] = 0x00;
				}

				if( i >= 120 && i < 160){
					// green
					screenBuf[i][j][0] = 0x05; 
					screenBuf[i][j][1] = 0xE0;
				}

			}

		// Call RamWrite
		command[0] = 0x2c;
		pwrite(spifd, &command, 1, 0);
		sleep(500);
		
		fprint(gpio, "set 25 1");
		//Mashes all memory to be 0, which should be white


		for(i=0;i<160;i++)
			for(j=0;j<128;j++){
				command[0] = screenBuf[i][j][0]; 
				command[1] = screenBuf[i][j][1];
				pwrite(spifd, &command, 2, 0);
			}

		fprint(gpio, "set 25 0");
		sleep(1000);


	 	for(i=0;i<160;i++)
			for(j=0;j<128;j++){

				if(i >= 0 && i < 40){
					// green
					screenBuf[i][j][0] = 0x05; 
					screenBuf[i][j][1] = 0xE0;
				}

				if(i >= 40 && i < 80){
					// Red 
					screenBuf[i][j][0] = 0xF8; 
					screenBuf[i][j][1] = 0x00; 
				}

				if(i >= 80 && i < 120){
					// Blue
					screenBuf[i][j][0] = 0x00; 
					screenBuf[i][j][1] = 0x1F;
				}

				if( i >= 120 && i < 160){
					// Orange 
					screenBuf[i][j][0] = 0xFC; 
					screenBuf[i][j][1] = 0x00;
				}

			}


		// Call RamWrite
		command[0] = 0x2c;
		pwrite(spifd, &command, 1, 0);
		sleep(500);
		
		fprint(gpio, "set 25 1");
		//Mashes all memory to be 0, which should be white


		for(i=0;i<160;i++)
			for(j=0;j<128;j++){
				command[0] = screenBuf[i][j][0]; 
				command[1] = screenBuf[i][j][1];
				pwrite(spifd, &command, 2, 0);
			}

		fprint(gpio, "set 25 0");
		sleep(1000);


		 for(i=0;i<160;i++)
			for(j=0;j<128;j++){

				if(i >= 0 && i < 40){
					// Orange 
					screenBuf[i][j][0] = 0xFC; 
					screenBuf[i][j][1] = 0x00;
				}

				if(i >= 40 && i < 80){
					// green
					screenBuf[i][j][0] = 0x05; 
					screenBuf[i][j][1] = 0xE0; 
				}

				if(i >= 80 && i < 120){
					// Red 
					screenBuf[i][j][0] = 0xF8; 
					screenBuf[i][j][1] = 0x00;
				}

				if( i >= 120 && i < 160){
					// Blue
					screenBuf[i][j][0] = 0x00; 
					screenBuf[i][j][1] = 0x1F;
				}

			}


		// Call RamWrite
		command[0] = 0x2c;
		pwrite(spifd, &command, 1, 0);
		sleep(500);
		
		fprint(gpio, "set 25 1");
		//Mashes all memory to be 0, which should be white


		for(i=0;i<160;i++)
			for(j=0;j<128;j++){
				command[0] = screenBuf[i][j][0]; 
				command[1] = screenBuf[i][j][1];
				pwrite(spifd, &command, 2, 0);
			}

		fprint(gpio, "set 25 0");
		sleep(1000);

		 for(i=0;i<160;i++)
			for(j=0;j<128;j++){

				if(i >= 0 && i < 40){
					// Blue
					screenBuf[i][j][0] = 0x00; 
					screenBuf[i][j][1] = 0x1F;

				}

				if(i >= 40 && i < 80){
					// Orange 
					screenBuf[i][j][0] = 0xFC; 
					screenBuf[i][j][1] = 0x00; 
				}

				if(i >= 80 && i < 120){
					// green
					screenBuf[i][j][0] = 0x05; 
					screenBuf[i][j][1] = 0xE0;
				}

				if( i >= 120 && i < 160){
					// Red 
					screenBuf[i][j][0] = 0xF8; 
					screenBuf[i][j][1] = 0x00;
				}

			}


		// Call RamWrite
		command[0] = 0x2c;
		pwrite(spifd, &command, 1, 0);
		sleep(500);
		
		fprint(gpio, "set 25 1");
		//Mashes all memory to be 0, which should be white


		for(i=0;i<160;i++)
			for(j=0;j<128;j++){
				command[0] = screenBuf[i][j][0]; 
				command[1] = screenBuf[i][j][1];
				pwrite(spifd, &command, 2, 0);
			}

		fprint(gpio, "set 25 0");
		sleep(1000);
	}



	



}	
