

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>

#define WISONIC_DEBUG
#undef WISONIC_DEBUG
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if( tcgetattr( fd,&oldtio)  !=  0) {
	    perror("tcgetattr error");
	    return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD; 
	newtio.c_cflag &= ~CSIZE; 

	switch( nBits )
	{
	    case 7:
	        newtio.c_cflag |= CS7;
	        break;
	    case 8:
	        newtio.c_cflag |= CS8;
	        break;
	}

	switch( nEvent )
	{
	    case 'O':
	        newtio.c_cflag |= PARENB; 
	        newtio.c_cflag |= PARODD;  
	        newtio.c_iflag |= (INPCK | ISTRIP); 
	        break;
	    case 'E':
                newtio.c_iflag |= (INPCK | ISTRIP);
                newtio.c_cflag |= PARENB;
                newtio.c_cflag &= ~PARODD;
                break;
	    case 'N': 
	        newtio.c_cflag &= ~PARENB;
	        break;
	}

	switch( nSpeed )
	{
	    case 2400:
	        cfsetispeed(&newtio, B2400);
	        cfsetospeed(&newtio, B2400);
	        break;
	    case 4800:
	        cfsetispeed(&newtio, B4800);
	        cfsetospeed(&newtio, B4800);
	        break;
	    case 9600:
	        cfsetispeed(&newtio, B9600);
	        cfsetospeed(&newtio, B9600);
	        break;
	    case 115200:
	        cfsetispeed(&newtio, B115200);
	        cfsetospeed(&newtio, B115200);
	        break;
	    case 460800:
	        cfsetispeed(&newtio, B460800);
	        cfsetospeed(&newtio, B460800);
	        break;
	    default:
	        cfsetispeed(&newtio, B9600);
	        cfsetospeed(&newtio, B9600);
	        break;
	}

	if( nStop == 1){
	    newtio.c_cflag &=  ~CSTOPB; 
	}else if ( nStop == 2 ){
	    newtio.c_cflag |=  CSTOPB;
	} 
	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH); 
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
	    perror("set error");
	    return -1;
	}
	return 0;
}

int main(int argc,char *argv[])
{
	int fd,ret_set,ret_read,ret;
	char buf_read[50];
	if(4 == argc)
	{

	    fd = open(argv[1], O_RDWR);
	    if(fd == -1)
	    {
		printf("Open %s failed! Exit!\n",argv[1]);
		exit(1);
	    }
	    #ifdef WISONIC_DEBUG
	    printf("open %s successfully!\n",argv[1]);
	    #endif
	    ret_set = set_opt(fd, atoi(argv[2]), 8, 'N', 1);
	    if (ret_set == -1)
	    {
		printf("Set %s failed! Exit!\n",argv[1]);
		exit(1);
	    }
            #ifdef WISONIC_DEBUG
	    printf("Set %s successfully!\n",argv[1]);
	    printf("Baud rate: %s\n",argv[2]);	
            printf("Data: %s\n",argv[3]);
	    #endif		
	    do
	    { 
	        memset(buf_read, 0, sizeof(buf_read));
        	tcflush(fd,TCIOFLUSH);

	        ret = write(fd, argv[3],strlen(argv[3])+1);
                if( ret > 0){
		    printf("Write data: %s:len:%d\n",argv[3],ret);
	        }else{
		    #ifdef WISONIC_DEBUG
		    printf("Write data failed! Exit!\n");
		    exit(1);
		    #endif
                }
		sleep(1);
	        ret_read = read(fd, buf_read, 100);
	        if(ret_read > 0){
	            printf("Read data: %s,size=%d\n\n", buf_read,ret_read);
	        }else{
		    #ifdef WISONIC_DEBUG
		    printf("read data failed! Exit!\n");
		    exit(1);
		    #endif
                }
	        //sleep(3);
	    }while (0);
            close(fd);
	}else{
	    printf("Usage: uart [tty node] [baud rate] [data]\n");
	    printf("       Sample: uart ttyHSL1 115200 test\n");
	}
 return 0;
}
