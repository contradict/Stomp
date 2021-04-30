/*
   spidevlib.c - A user-space program to comunicate using spidev.
   Gustavo Zamboni
   */
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

//////////
// Init SPIdev
//////////
int spi_init(char filename[40])
{
	int file;
	__u8    mode, lsb, bits;
	__u32 speed=2500000;

	if ((file = open(filename,O_RDWR)) < 0)
	{
		printf("Failed to open the bus.");
		/* ERROR HANDLING; you can check errno to see what went wrong */
		exit(1);
	}

	///////////////
	// Verifications
	///////////////
	//possible modes: mode |= SPI_LOOP; mode |= SPI_CPHA; mode |= SPI_CPOL; mode |= SPI_LSB_FIRST; mode |= SPI_CS_HIGH; mode |= SPI_3WIRE; mode |= SPI_NO_CS; mode |= SPI_READY;
	//multiple possibilities using |
	mode = 0;
	if (ioctl(file, SPI_IOC_WR_MODE, &mode)<0)   {
		perror("can't set spi mode");
		return -1;
	}

	if (ioctl(file, SPI_IOC_RD_MODE, &mode) < 0)
	{
		perror("SPI rd_mode");
		return -1;
	}
	if (ioctl(file, SPI_IOC_RD_LSB_FIRST, &lsb) < 0)
	{
		perror("SPI rd_lsb_fist");
		return -1;
	}
	if (ioctl(file, SPI_IOC_WR_BITS_PER_WORD, (__u8[1]){8})<0)   
	{
		perror("can't set bits per word");
		return -1;
	}
	if (ioctl(file, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) 
	{
		perror("SPI bits_per_word");
		return -1;
	}
	speed = 2500000;
	if (ioctl(file, SPI_IOC_WR_MAX_SPEED_HZ, &speed)<0)  
	{
	   perror("can't set max speed hz");
	   return -1;
	}
	if (ioctl(file, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) 
	{
		perror("SPI max_speed_hz");
		return -1;
	}


	printf("%s: spi mode %d, %d bits %sper word, %d Hz max\n",filename, mode, bits, lsb ? "(lsb first) " : "", speed);

	return file;
}



//////////
// Read n bytes from the 2 bytes add1 add2 address
//////////

//char * spi_read(int add1,int add2,int nbytes,int file)
//{
//    int status;
//
//    memset(buf, 0, sizeof buf);
//    memset(buf2, 0, sizeof buf2);
//    buf[0] = 0x01;
//    buf[1] = add1;
//    buf[2] = add2;
//    xfer[0].tx_buf = (unsigned long)buf;
//    xfer[0].len = 3; /* Length of  command to write*/
//    xfer[1].rx_buf = (unsigned long) buf2;
//    xfer[1].len = nbytes; /* Length of Data to read */
//    status = ioctl(file, SPI_IOC_MESSAGE(2), xfer);
//    if (status < 0)
//    {
//        perror("SPI_IOC_MESSAGE");
//        return NULL;
//    }
//    //printf("env: %02x %02x %02x\n", buf[0], buf[1], buf[2]);
//    //printf("ret: %02x %02x %02x %02x\n", buf2[0], buf2[1], buf2[2], buf2[3]);
//
//    return buf2;
//}

//////////
// Write n bytes int the 2 bytes address add1 add2
//////////
//void spi_write(int add1,int add2,int nbytes,char value[10],int file)
//{
//    unsigned char   buf[32], buf2[32];
//    int status;
//
//    memset(buf, 0, sizeof buf);
//    memset(buf2, 0, sizeof buf2);
//    buf[0] = 0x00;
//    buf[1] = add1;
//    buf[2] = add2;
//    if (nbytes>=1) buf[3] = value[0];
//    if (nbytes>=2) buf[4] = value[1];
//    if (nbytes>=3) buf[5] = value[2];
//    if (nbytes>=4) buf[6] = value[3];
//    xfer[0].tx_buf = (unsigned long)buf;
//    xfer[0].len = nbytes+3; /* Length of  command to write*/
//    status = ioctl(file, SPI_IOC_MESSAGE(1), xfer);
//    if (status < 0)
//    {
//        perror("SPI_IOC_MESSAGE");
//        return;
//    }
//    //printf("env: %02x %02x %02x\n", buf[0], buf[1], buf[2]);
//    //printf("ret: %02x %02x %02x %02x\n", buf2[0], buf2[1], buf2[2], buf2[3]);
//
//}
