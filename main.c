#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>


#define CRTSCTS  020000000000

int set_interface_attribs (int fd, int speed, int parity);
void set_blocking (int fd, int should_block);


int main()
{
    char *portname = "/dev/ttyUSB0";
    unsigned char cmd[] = "command_string";

    char response [256];
    int spot = 0, n = 0;
    int i = 0;
    char buf = '\0';
    int fd = open (portname, O_RDWR| O_NOCTTY);
    if (fd < 0)
    {
            printf("error %d opening %s: %s", errno, portname, strerror (errno));
            return -1;
    }

    set_interface_attribs (fd, B4800, 0);
    set_blocking (fd, 0);                


    printf("Sending request.\n");
    write(fd, &cmd, sizeof cmd );        

    usleep(2000); // sleep a bit before reading response

    n = 0,
    spot = 0;
    buf = '\0';
    memset(response, '\0', sizeof response);

    do
    {
        n = read(fd, &buf, 1);
        sprintf(&response[spot], "%c", buf);
        spot += n;
    } while (buf != '\r' && n > 0);

    // print out the response
    printf("%s\n", response);
    sleep(1);

}


int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag     &=  ~PARENB;            // Make 8n1
        tty.c_cflag     &=  ~CSTOPB;
        tty.c_cflag     &=  ~CSIZE;
        tty.c_cflag     |=  CS8;

        tty.c_cflag     &=  ~CRTSCTS;           // no flow control
        tty.c_cc[VMIN]   =  1;                  // read doesn't block
        tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
        tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

        cfmakeraw(&tty);

        tcflush(fd, TCIFLUSH);

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                printf("error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                printf ("error %d setting term attributes", errno);
}

