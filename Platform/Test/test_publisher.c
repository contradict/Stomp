
#include <stdio.h>
#include <unistd.h>

#include "messages.h"
#include "telem.h"

int main(int argc, char **argv)
{
    telem_init();
    struct tank_psi msg;
    while (1) {
        msg.psi = 47;
        printf("Publishing!\n");
        telem_publish(TANK_PSI, (char *)&msg, sizeof(msg));
        sleep(1);
    }

    return 0;
}