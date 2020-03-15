#include <stdio.h>
#include "rfd900x.h"

#include "telem.h"

void telem_init()
{
    rfd900x_init();
}

void telem_publish(msg_id id, char* data, size_t size)
{
    struct tank_psi * msg = (struct tank_psi *)data;
    printf("Received %d tank psi %d\n", id, msg->psi);
}