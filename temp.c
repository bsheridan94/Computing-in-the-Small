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
    long time_nsec;
    long time_elaps;

    for(i = 0; i < 8; i++){
        //Send a 1
        if(cmd & 0x01){
            time_nsec = nsec();
            time_elaps = nsec();
            fprint(gpioFD, "function 27 pulse");
            while((time_elaps - time_nsec) < 80000){
                time_elaps = nsec();
            }
        }
        //Send a 0
        else{
            time_nsec = nsec();
            time_elaps = nsec();
            fprint(gpioFD, "function 27 out");
            while((time_elaps - time_nsec) < 60000){
                time_elaps = nsec();
            }
            fprint(gpioFD, "function 27 in");
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
        fprint(gpioFD, "function 27 pulse");
        read(gpioFD, buf, 16);   
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

    status = resetWire();
    writeWire(0xCC);
    writeWire(0xBE);

    for(i = 0; i < 9; i++) {
        temp[i] = readWire();
    }
 
    print("temp[0]: %02x\n", temp[0]);
    print("temp[1]: %02x\n", temp[1]);
    print("temp[2]: %02x\n", temp[2]);
    print("temp[3]: %02x\n", temp[3]);
    print("temp[4]: %02x\n", temp[4]);
    print("temp[5]: %02x\n", temp[5]);
    print("temp[6]: %02x\n", temp[6]);
    print("temp[7]: %02x\n", temp[7]);
    print("temp[8]: %02x\n", temp[8]);


    final = temp[1] << 8 | temp[0];

    print("temp: %d\n", final>>4);

}
