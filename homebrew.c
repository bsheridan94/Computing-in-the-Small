#include <u.h>
#include <libc.h>

#define SOFTWARE_REST 0x01
#define SLEEP_OUT 0x11
#define PARTIAL_OFF 0x13
#define DISPLAY_ON 0x29



int resetWire(void);
void writeWire(int);
uchar readWire(void);
void initScreen(void);
void initBuses(void);
void writeColorsToScreen(void);
ushort readTempSense(void);

void initFridgeHeater(void);

void turnOffHeater(void);
void turnOnHeater(void);

void turnOffFridge(void);
void turnOnFridge(void);

void writeFirstDigit(int digit);
void writeSecondDigit(int digit);

void writeBufferToScreen(void);


	/*
	Screen Wiring

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


	/*
	
	Fridge & Heater Wiring

	Heater Pin 22
	Freezer Pin 16

	*/

	/*

	Temp Sensor - ds18b20

	Power - 5v
	Data - pin 27
	Gnd - GND

	*/


	/* GLOBALS */


	// Command array
	uchar command[32];
	//Stores data to write to screen
	int screenBuf[160][128][2];

	// File Descriptors
	int spifd;
	int gpiofd;

		int zeroTemplate[11][8] = {
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
		};

		int oneTemplate[11][8] = {
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
		};

		int twoTemplate[11][8] = {
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
		};

		int threeTemplate[11][8] = {
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
			

		};

		int fourTemplate[11][8] = {
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
			
		};

		int fiveTemplate[11][8] = {
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
			
		};
		
		int sixTemplate[11][8] = {
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
			
		};

		int sevenTemplate[11][8] = {
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
			
		};

		int eightTemplate[11][8] = {
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
			
		};

		int nineTemplate[11][8] = {
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff},
			{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
			{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
			
		};


void
displayTemp(ushort temp){

	int i;
	int j;

         for(i=0;i<160;i++)
			for(j=0;j<128;j++){
				screenBuf[i][j][0] = 0xFF;
				screenBuf[i][j][1] = 0xFF;
			}

		if(temp > 99)
			temp = 99;

		writeFirstDigit(temp);
		writeSecondDigit(temp);
		writeBufferToScreen();
}

void
writeFirstDigit(int digit){
	
	int color = 0x00;


	int i;
		int j;
		int k;
		int l;


	switch(digit){
		case 0:
			for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(zeroTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+l][0] = color;
					}
				}
		break;

		case 1:
			for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(oneTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+l][0] = color;
					}
				}
		break;

		case 2:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(twoTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+l][0] = color;
					}
				}
		break;

		case 3:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(threeTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+l][0] = color;
					}
				}
		break;

		case 4:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(fourTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+l][0] = color;
					}
				}
		break;

		case 5:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(fiveTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+l][0] = color;
					}
				}
		break;

		case 6:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(sixTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+l][0] = color;
					}
				}
		break;

		case 7:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(sevenTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+l][0] = color;
					}
				}
		break;

		case 8:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(eightTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+l][0] = color;
					}
				}
		break;

		case 9:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(nineTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+l][0] = color;
					}
				}
		break;

		default:
		break;
	}
}



void
writeSecondDigit(int digit){
	
	int color = 0x00;


	int i;
		int j;
		int k;
		int l;


	switch(digit){
		case 0:
			for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(zeroTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+64+l][0] = color;
					}
				}
		break;

		case 1:
			for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(oneTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+64+l][0] = color;
					}
				}
		break;

		case 2:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(twoTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+64+l][0] = color;
					}
				}
		break;

		case 3:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(threeTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+64+l][0] = color;
					}
				}
		break;

		case 4:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(fourTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+64+l][0] = color;
					}
				}
		break;

		case 5:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(fiveTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+64+l][0] = color;
					}
				}
		break;

		case 6:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(sixTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+64+l][0] = color;
					}
				}
		break;

		case 7:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(sevenTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+64+l][0] = color;
					}
				}
		break;

		case 8:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(eightTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+64+l][0] = color;
					}
				}
		break;

		case 9:
		for(i = 0; i < 12; i++)
				for(j = 0; j < 9; j++ ){
					if(nineTemplate[i][j] == 0x00){
						for(k = 0; k < 8; k++)
							for(l = 0; l < 8; l++)
								screenBuf[i*8+k][j*8+64+l][0] = color;
					}
				}
		break;

		default:
		break;
	}
}



void
writeBufferToScreen(){
		int i,j;
			// Call RamWrite
		command[0] = 0x2c;
		pwrite(spifd, &command, 1, 0);
		sleep(500);
		
		fprint(gpiofd, "set 25 1");
		//Mashes all memory to be 0, which should be white


		for(i=0;i<160;i++)
			for(j=0;j<128;j++){
				command[0] = screenBuf[i][j][0]; 
				command[1] = screenBuf[i][j][1];
				pwrite(spifd, &command, 2, 0);
			}

		fprint(gpiofd, "set 25 0");
		sleep(1000);
}

void
initBuses(){
	// Initialize SPI Device
 	spifd = open("/dev/spi0",ORDWR);
	if(spifd < 0){
		bind("#π","/dev",MAFTER);
		spifd = open("/dev/spi0",ORDWR);
		if(spifd < 0){
			print("error with SPI");
		}
	}

	// Initialize SPI Device
 	gpiofd = open("/dev/gpio",ORDWR);
	if(gpiofd < 0){
		bind("#G","/dev",MAFTER);
		gpiofd = open("/dev/gpio",ORDWR);
		if(spifd < 0){
			print("error with gpio");
		}
	}

}


void 
turnOnHeater(){
	fprint(gpiofd, "function 22 out");
	fprint(gpiofd, "set 22 0");
}

void
turnOffHeater(){
	fprint(gpiofd, "function 22 out");
	fprint(gpiofd, "set 22 1");
}

void 
turnOnFridge(){
	fprint(gpiofd, "function 17 out");
	fprint(gpiofd, "set 17 0");
}

void
turnOffFridge(){
	fprint(gpiofd, "function 17 out");
	fprint(gpiofd, "set 17 1");
}

void
initFridgeHeater(){
	//Heater
	fprint(gpiofd, "function 22 out");
	turnOffHeater();

	//Fridge
	fprint(gpiofd, "function 17 out");
	turnOnFridge();
}


void
initScreen(){

	int i = 0;
	int j = 0;

	// Initializes screen to black
    for(i=0;i<160;i++)
		for(j=0;j<128;j++){
			screenBuf[i][j][0] = 0x00; 
			screenBuf[i][j][1] = 0x00; 
		}
	

	//Set D/C
	// 1 data
	// 0 command
	fprint(gpiofd, "function 25 out");
	fprint(gpiofd, "set 25 0");

	//Set Reset 
	fprint(gpiofd, "function 24 out");
	fprint(gpiofd, "set 24 1");

	//Set CS
	fprint(gpiofd, "function 23 out");
	fprint(gpiofd, "set 23 0");



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

	fprint(gpiofd, "set 25 1");
	command[0] = 0x05;
	pwrite(spifd, &command, 1, 0);
	sleep(500);
	fprint(gpiofd, "set 25 0");
	

	// Set Cols 
	command[0] = 0x2a;
	pwrite(spifd, &command, 1, 0);
	sleep(500);

	fprint(gpiofd, "set 25 1");
	command[0] = 0x00;
	command[1] = 0x00;
	command[2] = 0x00;
	command[3] = 127;
	pwrite(spifd, &command, 4, 0);
	sleep(500);
	fprint(gpiofd, "set 25 0");

	//Set Rows 
	command[0] = 0x2b;
	pwrite(spifd, &command, 1, 0);
	sleep(500);

	fprint(gpiofd, "set 25 1");
	command[0] = 0x00;
	command[1] = 0x01;
	command[2] = 0x00;
	command[3] = 159;
	pwrite(spifd, &command, 4, 0);
	sleep(500);
	fprint(gpiofd, "set 25 0");

	// Call RamWrite
	command[0] = 0x2c;
	pwrite(spifd, &command, 1, 0);
	sleep(500);
	
	fprint(gpiofd, "set 25 1");
	//Mashes all memory to be 0, which should be white

	//Write current ScreenBuf to memory
	for(i=0;i<160;i++)
		for(j=0;j<128;j++){
			command[0] = screenBuf[i][j][0]; 
			command[1] = screenBuf[i][j][1];
			pwrite(spifd, &command, 2, 0);
		}

	fprint(gpiofd, "set 25 0");
	sleep(1000);


}

	/*
		The code writes 4 giants stripes to the screen. Each Stripe is a different color. 
		Each second the screen is rewritten, and each Stripe is cycled down one. THe bottom is then moved back on 
		to the top. The code runs forever generating the animation.

		The use of the screenbuf allows the program to draw what ever it would like through accessing the array and writing the RGB 5-6-5 code to it.
		This stripe animation proves the program has full control of the screen.

	*/

void
writeColorsToScreen(){
	int i;
	int j;
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
		
		fprint(gpiofd, "set 25 1");
		//Mashes all memory to be 0, which should be white


		for(i=0;i<160;i++)
			for(j=0;j<128;j++){
				command[0] = screenBuf[i][j][0]; 
				command[1] = screenBuf[i][j][1];
				pwrite(spifd, &command, 2, 0);
			}

		fprint(gpiofd, "set 25 0");
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
		
		fprint(gpiofd, "set 25 1");
		//Mashes all memory to be 0, which should be white


		for(i=0;i<160;i++)
			for(j=0;j<128;j++){
				command[0] = screenBuf[i][j][0]; 
				command[1] = screenBuf[i][j][1];
				pwrite(spifd, &command, 2, 0);
			}

		fprint(gpiofd, "set 25 0");
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
		
		fprint(gpiofd, "set 25 1");
		//Mashes all memory to be 0, which should be white


		for(i=0;i<160;i++)
			for(j=0;j<128;j++){
				command[0] = screenBuf[i][j][0]; 
				command[1] = screenBuf[i][j][1];
				pwrite(spifd, &command, 2, 0);
			}

		fprint(gpiofd, "set 25 0");
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
		
		fprint(gpiofd, "set 25 1");
		//Mashes all memory to be 0, which should be white


		for(i=0;i<160;i++)
			for(j=0;j<128;j++){
				command[0] = screenBuf[i][j][0]; 
				command[1] = screenBuf[i][j][1];
				pwrite(spifd, &command, 2, 0);
			}

		fprint(gpiofd, "set 25 0");
		sleep(1000);
	}

int resetWire()
{
    ulong x;
    char buf[16];
    long time_nsec;
    long time_elaps;


    fprint(gpiofd, "function 27 out");

    // wait 500 us  
    time_nsec = nsec();
    time_elaps = nsec();
    while( (time_elaps - time_nsec) < 600000){
        time_elaps = nsec();
    }

    //set data line to input
    fprint(gpiofd, "function 27 in");
    // wait 60 us
    while((time_elaps - time_nsec) < 660000){
        time_elaps = nsec();
    }


    read(gpiofd, buf, 16);   
    buf[8] = 0;
    x =  strtoull(buf ,nil, 16);


    if(!(x &  (1<<27))){
        //print("Response Received!\n");
        return 0;
    }
    else{
        print("Reset error\n");
        return 1;
    }
}


void writeWire(int cmd){
    int i;
    long time_nsec;
    long time_elaps;

    for(i = 0; i < 8; i++){
        //Send a 1
        if(cmd & 0x01){
            time_nsec = nsec();
            time_elaps = nsec();
            fprint(gpiofd, "function 27 pulse");
            while((time_elaps - time_nsec) < 80000){
                time_elaps = nsec();
            }
        }
        //Send a 0
        else{
            time_nsec = nsec();
            time_elaps = nsec();
            fprint(gpiofd, "function 27 out");
            while((time_elaps - time_nsec) < 60000){
                time_elaps = nsec();
            }
            fprint(gpiofd, "function 27 in");
            while((time_elaps - time_nsec) < 80000){
                time_elaps = nsec();
            }
        }
        cmd >>= 1;//we move to the next bit
    }
}


uchar readWire(){
    ulong x;
    int i;
    uchar value;
    char buf[16];

    long time_nsec;
    long time_elaps;

    value = 0;

    //for(i = 7; i > -1; i--){
    for(i = 0; i < 8; i++){
        time_nsec = nsec();
        time_elaps = nsec();
        fprint(gpiofd, "function 27 pulse");
        read(gpiofd, buf, 16);   
        buf[8] = 0;
        x = strtoull(buf, nil, 16);
        if(x & (1<<27)){
            value |= (1 << i);
        }
        while((time_elaps - time_nsec) < 60000){
                time_elaps = nsec();
        }
    }

    //print("result inside read call: %d\n", value);
    return value;
}

float convertTemp(){
	float result = 0.0;

	return result;
}

ushort readTempSense(void){
	int status;
    int i;
    uchar temp[9];
    ushort finalTemp;
    static int firstTime = 0;

    fprint(gpiofd, "function 27 in");
        fprint(gpiofd, "set 27 0");


        status = resetWire();
        sleep(1000);

        writeWire(0xCC);
        writeWire(0x44);
        sleep(1000);

        status = resetWire();
        sleep(1000);
        writeWire(0xCC);
        writeWire(0xBE);

        for(i = 0; i < 9; i++) {
            temp[i] = readWire();
        }
         
        if(firstTime){
        print("temp[0]: %02x\n", temp[0]);
        print("temp[1]: %02x\n", temp[1]);
        print("temp[2]: %02x\n", temp[2]);
        print("temp[3]: %02x\n", temp[3]);
        print("temp[4]: %02x\n", temp[4]);
        print("temp[5]: %02x\n", temp[5]);
        print("temp[6]: %02x\n", temp[6]);
        print("temp[7]: %02x\n", temp[7]);
        print("temp[8]: %02x\n", temp[8]);
        firstTime = 0;
        }

        finalTemp = temp[1] << 8 | temp[0];
        print("temp: %d C\n", finalTemp >> 4);

        return finalTemp;
}

void
main()
{
	ushort temp;
	initBuses();
	initFridgeHeater();
	initScreen();
		while(1){
			
			turnOnHeater();

			//writeColorsToScreen();
			temp = readTempSense();
			displayTemp(temp);
			turnOffHeater();
			sleep(3000);
			displayTemp(0);
			sleep(500);
			displayTemp(1);
			sleep(500);
			displayTemp(2);
			sleep(500);
			displayTemp(3);
			sleep(500);
			displayTemp(4);
			sleep(500);
			displayTemp(5);
			sleep(500);
			displayTemp(6);
			sleep(500);
			displayTemp(7);
			sleep(500);
			displayTemp(8);
			sleep(500);
			displayTemp(9);
			


			//FINITE STATE MACHINE
			// if((temp << 2) == 22){
			// 	turnOffFridge();
			// }

		}

}
