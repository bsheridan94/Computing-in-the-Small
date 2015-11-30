#include<u.h>
#include<libc.h>


int resetWire(void);
void writeWire(int);
uchar readWire(void);


int gpioFD;

//function to reset write
int resetWire()
{
	ulong x;
    char buf[16];
	ulong time;
    time = nsec();
    //set data line to output
    fprint(gpioFD, "function 27 out");

    // wait 500 us	
    while(nsec() < (time + 500000));
    //set data line to input
    fprint(gpioFD, "function 27 in");

    // wait 60 us
	while(nsec() < (time + 560000));
    read(gpioFD, buf, 16);

    x =  strtoul(buf ,nil, 16);

    if(!(x &  (1<<27))){
    	print("Response Received!\n");
    	return 0;
    }
    else{
    	print("Reset error\n");
    	return 1;
    }
}


void writeWire(int cmd){
    int i;
    ulong time;

    for(i = 0; i < 8; i++){
        //Send a 1
        if(cmd & 0x01){
            time = nsec();
            fprint(gpioFD, "function 27 pulse");
            while(nsec() < (time + 60000));
        }
        //Send a 0
        else{
            time = nsec();
            fprint(gpioFD, "function 27 out");
            while(nsec() < (time + 60000));
            fprint(gpioFD, "function 27 in");
            while(nsec() < (time + 80000));
        }
        cmd >>= 1;//we move to the next bit
    }
}


uchar readWire(){
    ulong x;
    int i;
    uchar value;
    char buf[16];
    ulong time;

    value = 0;

    for(i = 0; i < 8; i++){
        time = nsec();
        fprint(gpioFD, "function 27 pulse");

        x = strtoul(buf, nil, 16);
        if(x & (1<<27)){
            value |= (1 << i);
        }
        print("%d ", x & (1<<27));
        while(nsec() < (time + 60000));
    }

    print("\nresult inside read call: %u\n", value);
    return value;
}


void main(){
	int status;
    int i;
    uchar temp[9];
    ushort final;

	gpioFD = open("/dev/gpio",ORDWR);
    if(gpioFD < 0) {
        bind("#G","/dev",MAFTER);
        gpioFD = open("/dev/gpio",ORDWR);
        if(gpioFD < 0)
        {
            print("Open Error: %r\n");
        }
    }
    
    //set pin function and value
    fprint(gpioFD, "function 27 in");
    fprint(gpioFD, "set 27 0");

    status = resetWire();
    sleep(500);

    writeWire(0xCC);
    writeWire(0x44);
    sleep(1000);

    status = ReadWire();
    writeWire(0xCC);
    writeWire(0x44);


    for(i = 0; i < 9; i++) {
        temp[i] = readWire();
    }
 
    print("temp[0]: %02x\n", temp[0]);
    print("temp[1]: %02x\n", temp[1]);

    final = temp[1] << 8 | temp[0];

    print("temp: %d\n", final);

}

