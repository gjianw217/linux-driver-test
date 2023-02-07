//wakeup pn532
//send
//55 55 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF 03 FD D4 14 01 17 00
//ack
//00 00 FF 00 FF 00 00 00 FF 02 FE D5 15 16 00
//0 0 ffffffff 0 ffffffff 0 0 0 ffffffff 2 fffffffe ffffffd5 15 16 0 
//send
//00 00 FF 04 FC D4 4A 01 00 E1 00 
//ack
//0x0 0x0 0xffffffff 0x0 0xffffffff 0x0 0x0 0x0 0xffffffff 0xc 0xfffffff4 0xffffffd5 0x4b 0x1 0x1 0x0 0x4 0x8 0x4 0x11 0x22 0x33 0x44 0x24 0x0 
#include<stdio.h>
#include<stdlib.h> 
#include<unistd.h>  
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h> 
#include<termios.h>
#include<errno.h>
#include<string.h> 
int main()
{
    int fd;
    int i;
    int len;
    int n = 0;      
    char read_buf[256];
    char write_buf[256];
    char wake[24]={0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0xfd, 0xd4, 0x14, 0x01, 0x17, 0x00};
    char search[11]={0x00, 0x00, 0xff, 0x04, 0xfc, 0xd4, 0x4a, 0x01, 0x00, 0xe1,0x00};
    struct termios opt; 
    fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);
    if(fd == -1)
    {
        perror("open serial 0\n");
        exit(0);
    }
    tcgetattr(fd, &opt);      
    bzero(&opt, sizeof(opt));
    tcflush(fd, TCIOFLUSH);
    cfsetispeed(&opt, B115200);
    cfsetospeed(&opt, B115200);
    opt.c_cflag &= ~CSIZE;  
    opt.c_cflag |= CS8;   
    opt.c_cflag &= ~CSTOPB; 
    opt.c_cflag &= ~PARENB; 
    opt.c_cflag &= ~CRTSCTS;
    opt.c_cflag |= (CLOCAL | CREAD);
    opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    opt.c_oflag &= ~OPOST;
    opt.c_cc[VTIME] = 0;
    opt.c_cc[VMIN] = 0;
    tcflush(fd, TCIOFLUSH);
    printf("configure complete\n");
    if(tcsetattr(fd, TCSANOW, &opt) != 0)
    {
        perror("serial error");
        return -1;
    }
    printf("wake start send and receive data\n");
    n = write(fd, wake, 24);
    do
    {    
        n = 0;
        len = 0;
        bzero(read_buf, sizeof(read_buf)); 
        sleep(5);
        while( (n = read(fd, read_buf, sizeof(read_buf))) > 0 )
        {
            len += n;
        }
	 
        printf("Len %d\n", len);
        for(i=0;i<len;i++)
       {
          printf("0x%x ",read_buf[i]);
       }
       sleep(2);
      
    }while(0);

    printf("search start send and receive data\n");
    n = write(fd, search, 11);
    do
    {    
        n = 0;
        len = 0;
        sleep(5);
        bzero(read_buf, sizeof(read_buf)); 
        while( (n = read(fd, read_buf, sizeof(read_buf))) > 0 )
        {
            len += n;
        }

        printf("Len %d \n", len);
        for(i=0;i<len;i++)
       {
          printf("0x%x ",read_buf[i]);
       }
       sleep(2);
    }while(0);

    return 0;   
}
