#include <u.h>
#include <libc.h>

// Brian Sheridan
// Lab 1
// CS680

/*
Used a manual switch and led with proper resisters and pull down resister
every time the button was pressed, the led would change modes.
OFF, ON, Blink
*/

void main(){

	// init variables
	int led;
	int state = 0;
	char buf[17];

	//binds the gpio
	bind("#G", "/dev", MAFTER);
	led = open("/dev/gpio", ORDWR);
	
	while(1){

	// mode 0 turns the led off
	if(state==0){
		fprint(led, "function 27 out");
		fprint(led, "set 27 0");
	}

	// mode 1 turns the led on
	if(state==1){
		fprint(led, "function 27 out");
		fprint(led, "set 27 1");
	}

	// mode 2 makes the led blink
	if(state==2){
	fprint(led, "function 27 out");
	fprint(led, "set 27 1");
	sleep(1000);
	fprint(led, "set 27 0");
	sleep(1000);
	}

	// reads in status of GPIO
	read(led, buf, 16);

	// DEBUG code that was used to check status of bits

	/*
	print("Status: ");
	print("%c\n",buf[0] );
	print("%c\n",buf[1] );
	print("%c\n",buf[2] );
	print("%c\n",buf[3] );
	print("%c\n",buf[4] );
	print("%c\n",buf[5] );
	print("%c\n",buf[6] );
	*/


	// Checks if button has been pressed
	sleep(100);
	int result =0;
	result = buf[2] & 0x4;
	print("button: %d\n", result);

	// Changes state if button is pressed, and implements a poor debouncing method with a sleep
	if(result == 4){
		state = state +1;
		if(state==3){
			state =0;
		}
	sleep(500);
	}


}
	
}
