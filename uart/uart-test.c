#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
typedef  unsigned int uint32_t ; 
static speed_t speed_arr[] = {B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B600, B300};
static int      name_arr[] = { 230400,  115200,  57600,  38400,  19200,  9600,  4800,  2400,  1200, 600,  300};
static int uart_fd;
/**
*@brief  设置串口通信速率
*@param  fd     类型 int  打开串口的文件句柄
*@param  speed  类型 int  串口速度
*@return  void
*/
static void set_speed(int fd, int speed){
	uint32_t  i; 
	struct termios opt;
	tcgetattr(fd, &opt); 
	tcflush(fd, TCIOFLUSH);     
	cfmakeraw(&opt);
	#if 1
	for(i= 0; i < sizeof(speed_arr)/sizeof(speed_t); i++) { 
		if  (speed == name_arr[i]) {     
			printf("serial speed=%d ", speed);
			cfsetispeed(&opt, speed_arr[i]);  
			cfsetospeed(&opt, speed_arr[i]);   
		}    
	}	
	#else
	cfsetispeed(&opt, B115200);
	cfsetospeed(&opt, B115200);
	#endif
	
	if (tcsetattr(fd, TCSANOW, &opt) == -1) {
		printf("tcsetattr(): %s", strerror(errno));
		return;
	}

	tcflush(fd,TCIOFLUSH);   
}

/**
*@brief   设置串口数据位，停止位和效验位
*@param  fd     类型  int  打开的串口文件句柄
*@param  databits 类型  int 数据位   取值 为 7 或者8
*@param  stopbits 类型  int 停止位   取值为 1 或者2
*@param  parity  类型  int  效验类型 取值为N,E,O,,S
*/
static int set_parity(int fd, int speed, int databits,char *parity,int stopbits)
{ 
	set_speed(fd,speed);
	struct termios options; 
	if  ( tcgetattr( fd,&options)  !=  0) { 
		perror("SetupSerial 1");     
		return -1;  
	}
	options.c_cflag &= ~CSIZE; 
	switch (databits) /*设置数据位数*/
	{   
		case 7:		
			options.c_cflag |= CS7; 
			break;
		case 8:     
			options.c_cflag |= CS8;
			break;   
		default:    
			fprintf(stderr,"Unsupported data size\n"); 
			return -1;  
	}
	printf("databits=%d ",databits);
	switch (parity[0]) 
	{   
		case 'n':
		case 'N':    
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
			break;  
		case 'o':   
		case 'O':     
			options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/  
			options.c_iflag |= INPCK;             /* Disnable parity checking */ 
			break;  
		case 'e':  
		case 'E':   
			options.c_cflag |= PARENB;     /* Enable parity */    
			options.c_cflag &= ~PARODD;   /* 转换为偶效验*/     
			options.c_iflag |= INPCK;       /* Disnable parity checking */
			break;
		case 'S': 
		case 's':  /*as no parity*/   
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;  
		default:   
		fprintf(stderr,"Unsupported parity\n");    
		return -1;  
	}  

	printf("parity=%c ",parity[0]);
	/* 设置停止位*/  
	switch (stopbits)
	{   
		case 1:    
			options.c_cflag &= ~CSTOPB;  
			break;  
		case 2:    
			options.c_cflag |= CSTOPB;  
			break;
		default:    
			fprintf(stderr,"Unsupported stop bits\n");  
			return -1; 
	} 

	printf("stopbits=%d\n",stopbits);
	/* Set input parity option */ 
	if (parity[0] != 'n')   
		options.c_iflag |= INPCK; 

	tcflush(fd,TCIFLUSH);
	//options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/   
	//options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0)   
	{ 
		perror("SetupSerial 3");   
		return -1;  
	} 
	return 0;  
}

static void uart_init(char * pserial_dev, int speed, int databits, char *parity, int stopbits)
{
	int ret = 0;
	printf("dev name is: %s \r\n", pserial_dev);
	uart_fd = open(pserial_dev, O_RDWR | O_NOCTTY);
	if (uart_fd < 0) {
		printf("open(): %s\r\n", strerror(errno));
		exit(1);
	}
	//set_parity(uart_fd, speed, 8, 'O', 1);
	ret = set_parity(uart_fd, speed, databits, parity, stopbits);
	if(ret) {
		printf("\noperating error!\r\n");
		close(uart_fd);
		exit(1);
	}
}

static int test_send(char * file_name)
{
	char buf[1024];
	int  fd, ret, tmp;
	fd = open(file_name, O_RDONLY);
	if(fd < 0) {
		printf("open %s faild!\n",file_name);
		return -1;
	}
	while(1) {
		ret = 0;
		tmp = 0;
		ret = read(fd, buf, sizeof(buf)); 
		if(ret <= 0) {
			//printf("read error!\n");
			break;
		}
		do {
			tmp += write(uart_fd, buf+tmp, ret-tmp);
		} while(tmp != ret);
	}

	close(fd);
	return 0;
}

static int test_receive(char * file_name)
{
	char buf[1024];
	int  fd, ret, tmp;
	fd = open(file_name,O_WRONLY|O_TRUNC|O_CREAT,0666);
	if(fd < 0) {
		printf("open %s faild!\n",file_name);
		return -1;
	}
	
	while(1) {
		ret = 0;
		tmp = 0;
		ret = read(uart_fd, buf, sizeof(buf)); 
		if(ret <= 0) {
			printf("read error!\n");
			break;
		}
		do {
			tmp += write(fd, buf+tmp, ret-tmp);
		} while(tmp != ret);
	}

	close(fd);
	return 0;
}

void print_info(void)
{
	int i;
	printf("\n./test_uart /dev/ttyX speed databits parity stopbits file-name transport\n\n");
	printf("	/dev/ttyX	/dev/ttySn or /dev/ttyUSBn /dev/ttyMAXn\n");
	printf("	speed		");
	for(i=0;name_arr[i];i++) {
		printf("%d ",name_arr[i]);
	}
	printf("\n");
	printf("	databits	8	7\n");
	printf("	parity		n	o	e\n");
	printf("	stopbits	1	2\n");
	printf("	file-name	./1.log\n");
	printf("	transport	send	receive\n");	
	printf("\n	E.g# ./test_uart /dev/ttyS1 115200 8 n 1 1.log receive\n\n");
}

int main(int argc,char *argv[])
{
	char * parity;
	int speed, databits, stopbits;
	if(argc != 8) {
		print_info();
		exit(1);
	}
	parity	 = argv[4];
	speed    = atoi(argv[2]);
	databits = atoi(argv[3]);
	stopbits = atoi(argv[5]);
	
	uart_init(argv[1],speed,databits,parity,stopbits);
	if(!strcmp(argv[7],"send")) 
	{
		test_send(argv[6]);
	} 
	else if(!strcmp(argv[7],"receive")) 
	{
		test_receive(argv[6]);
	}
	else
	{
		print_info();
	}
	
	close(uart_fd);
	return 0;
}
