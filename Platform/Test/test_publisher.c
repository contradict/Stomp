
#include <stdio.h>
#include <unistd.h>

#include "messages.h"
#include "telem.h"

int main(int argc, char **argv)
{
    telem_init();
    struct tank_psi msg;
    uint32_t psi = 0;
    while (1) {
        msg.psi = psi++;
        telem_publish(TANK_PSI, (char *)&msg, sizeof(msg));
        sleep(1);
    }

    return 0;
}