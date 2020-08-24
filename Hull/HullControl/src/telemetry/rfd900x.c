#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "rfd900x.h"

static int g_serial_port = -1;

static int set_interface_attributes(int fd, int speed, int parity)
{
    struct termios tty;
    if (tcgetattr (fd, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    tty.c_cflag |= (CLOCAL | CREAD);                // ignore modem controls,
    tty.c_cflag &= ~(PARENB | PARODD);              // shut off parity
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // shut off xon/xoff ctrl
    tty.c_iflag &= ~IGNBRK;                         // disable break processing
    tty.c_lflag = 0;                                // no signaling chars, no echo, no canonical processing
    tty.c_oflag = 0;                                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;                            // read doesn't block
    tty.c_cc[VTIME] = 5;                            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

void rfd900x_init()
{
    g_serial_port = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_SYNC);
    if (g_serial_port < 0)
    {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        return;
    }

    if (set_interface_attributes(g_serial_port, B115200, 0) < 0)
    {
        close(g_serial_port);
        g_serial_port = -1;
    }
}

void rfd900x_write(const char* data, size_t size)
{
    if (g_serial_port < 0)
    {
        printf("Invalid file descriptor - did you call rfd900x_init()?");
        return;
    }
    // TODO - validate size is something sane
    write(g_serial_port, data, size);
}
