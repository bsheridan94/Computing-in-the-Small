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
    long time_nsec;
    long time_elaps;

    fprint(gpioFD, "function 27 out");

    // wait 500 us  
    time_nsec = nsec();
    time_elaps = nsec();
    while( (time_elaps - time_nsec) < 600000){
        time_elaps = nsec();
    }

    //set data line to input
    fprint(gpioFD, "function 27 in");
    // wait 60 us
    while((time_elaps - time_nsec) < 660000){
        time_elaps = nsec();
    }


    read(gpioFD, buf, 16);   
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
            fprint(gpioFD, "function 27 pulse");
            // transactions should last 80us
            while((time_elaps - time_nsec) < 80000){
                time_elaps = nsec();
            }
        }
        //Send a 0
        else{
            time_nsec = nsec();
            time_elaps = nsec();
            fprint(gpioFD, "function 27 out");
            // waits 60us
            while((time_elaps - time_nsec) < 60000){
                time_elaps = nsec();
            }
            fprint(gpioFD, "function 27 in");
            // transactions should last 80us
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
    uchar ret;
    char buf[16];
    long time_nsec;
    long time_elaps;
    ret = 0;

    //for(i = 7; i > -1; i--){
    for(i = 0; i < 8; i++){
        time_nsec = nsec();
        time_elaps = nsec();
        fprint(gpioFD, "function 27 pulse");
        read(gpioFD, buf, 16);   
        buf[8] = 0;
        x = strtoull(buf, nil, 16);
        if(x & (1<<27)){
            ret |= (1 << i);
        }
        // transactions should last u0s
        while((time_elaps - time_nsec) < 60000){
                time_elaps = nsec();
        }
    }
    return ret;
}


void main(){
    int status;
    int i;
    uchar temp[9];
    ushort finalTemp;
    int firstTime = 1;

    //Binds GPIO
    gpioFD = open("/dev/gpio",ORDWR);
    if(gpioFD < 0) {
        bind("#G","/dev",MAFTER);
        gpioFD = open("/dev/gpio",ORDWR);
        if(gpioFD < 0)
        {
            print("Open Error: %r\n");
        }
    }

    while(1){
         //set pin function and value
        fprint(gpioFD, "function 27 in");
        fprint(gpioFD, "set 27 0");

        status = resetWire();
        sleep(1000);

        //Skip Rom
        writeWire(0xCC);
        // Convert Temp
        writeWire(0x44);
        sleep(1000);

        status = resetWire();
        sleep(1000);

        //skip Rom
        writeWire(0xCC);
        //read scratchPad
        writeWire(0xBE);

        // read in all 9 bytes of scratchpad
        for(i = 0; i < 9; i++) {
            temp[i] = readWire();
        }
         
         // prints whole scratch pad for first run
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

        // joins byte 1 and 0 of scratchpad to create temp.
        finalTemp = temp[1] << 8 | temp[0];
        print("temp: %d C\n", finalTemp >> 4);
        sleep(1000);
    }
}
