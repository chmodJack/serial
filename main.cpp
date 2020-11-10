
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

class serial
{
public:
	serial(const char* dev)
	{
		open(dev);
		save_old();
		set_new();
	}
	~serial(void)
	{
		restore_old();
	}

private:
	void open(const char* dev)
	{
		/* 
		   Open modem device for reading and writing and not as controlling tty
		   because we don't want to get killed if linenoise sends CTRL-C.
		 */
		fd = ::open(dev, O_RDWR | O_NOCTTY );
		if (fd <0)
		{
			perror(dev);
			exit(-1);
		}
	}

	void save_old(void)
	{
		tcgetattr(fd,&oldtio); /* save current serial port settings */
	}

	void set_new(void)
	{
		bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

		/* 
BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
CRTSCTS : output hardware flow control (only used if the cable has
all necessary lines. See sect. 7 of Serial-HOWTO)
CS8     : 8n1 (8bit,no parity,1 stopbit)
CLOCAL  : local connection, no modem contol
CREAD   : enable receiving characters
		 */

/* baudrate settings are defined in <asm/termbits.h>, which is
   included by <termios.h> */
#define BAUDRATE B115200

		newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

		/*
IGNPAR  : ignore bytes with parity errors
ICRNL   : map CR to NL (otherwise a CR input on the other computer
will not terminate input)
otherwise make device raw (no other input processing)
		 */
		newtio.c_iflag = IGNPAR | ICRNL;

		/*
		   Raw output.
		 */
		newtio.c_oflag = 0;

		/*
ICANON  : enable canonical input
disable all echo functionality, and don't send signals to calling program
		 */
		newtio.c_lflag = ICANON;

		/* 
		   initialize all control characters 
		   default values can be found in /usr/include/termios.h, and are given
		   in the comments, but we don't need them here
		 */
		newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
		newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
		newtio.c_cc[VERASE]   = 0;     /* del */
		newtio.c_cc[VKILL]    = 0;     /* @ */
		newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
		newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
		newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
		newtio.c_cc[VSWTC]    = 0;     /* '\0' */
		newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
		newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
		newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
		newtio.c_cc[VEOL]     = 0;     /* '\0' */
		newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
		newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
		newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
		newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
		newtio.c_cc[VEOL2]    = 0;     /* '\0' */

		//now clean the modem line and activate the settings for the port
		tcflush(fd, TCIFLUSH);
		tcsetattr(fd,TCSANOW,&newtio);
	}

	void restore_old(void)
	{
		/* restore the old port settings */
		tcsetattr(fd,TCSANOW,&oldtio);
	}

public:
	int fd=0;
private:
	struct termios oldtio;
	struct termios newtio;
};

#include<unistd.h>

int main(int argc,char* argv[])
{
	serial s("/dev/ttyUSB0");
	int fd = s.fd;

	int num=0;
	char arr[4096]={0};
	while(1)
	{
		//sleep(1);
		sprintf(arr,"hello [%d]\r\n",num);
		write(fd,arr,strlen(arr));
		num++;

		int cnt = read(fd,arr,4096);
		arr[cnt]='\0';
		printf("%d bytes read [%s]\r\n",cnt,arr);
	}
}
