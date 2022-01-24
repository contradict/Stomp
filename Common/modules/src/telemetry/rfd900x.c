#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "rfd900x.h"
#include "sclog4c/sclog4c.h"

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

static const char *tty_string = "/dev/ttyS2";

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static int s_serial_port = -1;

// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

static int set_interface_attributes(int fd, int speed)
{
    struct termios tty;
    if (tcgetattr (fd, &tty) != 0)
    {
        logm(SL4C_WARNING, "Error %i from tcgetattr: %s", errno, strerror(errno));
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);
 
    tty.c_cflag &=  ~PARENB;            // No Parity
    tty.c_cflag &=  ~CSTOPB;            // 1 Stop Bit
    tty.c_cflag &=  ~CSIZE;
    tty.c_cflag |=  CS8;                // 8 Bits
    tty.c_cflag |= (CLOCAL | CREAD);                // ignore modem controls,
    tty.c_cflag &= ~(PARENB | PARODD);              // shut off parity
    tty.c_cflag &= ~CRTSCTS;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // shut off xon/xoff ctrl
    tty.c_iflag &= ~IGNBRK;                         // disable break processing
    tty.c_lflag = 0;                                // no signaling chars, no echo, no canonical processing
    tty.c_oflag = 0;                                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;                            // read doesn't block
    tty.c_cc[VTIME] = 5;                            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        logm(SL4C_WARNING, "Error %i from tcsetattr: %s", errno, strerror(errno));
        return -1;
    }
    return 0;
}

void rfd900x_init()
{
    logm(SL4C_DEBUG, "Init RFD900x radio");

    s_serial_port = open(tty_string, O_RDWR | O_NOCTTY | O_SYNC);
    if (s_serial_port < 0)
    {
        logm(SL4C_FATAL, "Error %i from open(): %s", errno, strerror(errno));
        return;
    } else {
        logm(SL4C_DEBUG, "No error while opening port");
    }

    if (set_interface_attributes(s_serial_port, B57600) < 0)
    {
        close(s_serial_port);
        s_serial_port = -1;
    }

    logm(SL4C_DEBUG, "RFD900x init successful");
}

int rfd900x_get_fileno()
{
    return s_serial_port;
}

void rfd900x_write(const char* data, size_t size)
{
    if (s_serial_port < 0)
    {
        logm(SL4C_FATAL, "Invalid file descriptor - did you call rfd900x_init()?");
        return;
    }
    // TODO - validate size is something sane
    write(s_serial_port, data, size);
}

int rfd900x_read(char* buf, size_t buf_size)
{
    memset(buf, '\0', buf_size);
    int num_bytes = read(s_serial_port, buf, buf_size);

    //  Check to make sure we read the entire message

    if (num_bytes >= buf_size)
    {
       logm(SL4C_ERROR, "read s_serial_port filled the entire buffer");
    }

    logm(SL4C_FINE, "read s_serial_port returned %d bytes", num_bytes);
    lseek(s_serial_port, 0, SEEK_SET);

    return num_bytes;
}