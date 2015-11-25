#include <u.h>
#include <lib.c>


void
main(){

    int gpio_fd;

    bind("#G", "/dev",MAFTER);
    gpio_fd = open("/dev/gpio", ORDWR);

    fprint(gpio_fd, "function 25 out");
    fprint(gpio_fd, "set 25 0");

}
