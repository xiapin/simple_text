#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void print_usage(char *ch)
{
      printf("%s r : read at24c02 address 0\n",ch);
      printf("%s w val : write at24c02 address 0\n",ch);
}
void sys_error(char *ch)
{
      perror(ch);
      exit(1);
}

int main(int argc,char *argv[])
{
      int fd;
      unsigned char val;
      char register_addr = 0x08;/*addr*/
      int res;
      char wbuf[10];
      char rbuf[10];

      if(argc < 2){
	    print_usage(argv[0]);
	    exit(1);
      }

      if((fd = open("/dev/i2c_e2prom",O_RDWR)) < 0)
	    sys_error("open fd error\n");
      
      if(!strcmp(argv[1],"r"))
      {
	    /*先写入地址*/
	    if(write(fd,&register_addr,1) != 1)
		  sys_error("write fd failed\n");
	    if(read(fd,rbuf,5)!=5)
		  sys_error("read failed\n");
	    printf("rbuf[%d] = 0x%x\n",0,rbuf[0]);
	    printf("rbuf[%d] = 0x%x\n",1,rbuf[1]);
	    printf("rbuf[%d] = 0x%x\n",2,rbuf[2]);
	    printf("rbuf[%d] = 0x%x\n",3,rbuf[3]);
	    printf("rbuf[%d] = 0x%x\n",4,rbuf[4]);
      }
      else if(!strcmp(argv[1],"w"))
      {
	    val = strtoul(argv[2],NULL,0);

	    wbuf[0] = register_addr;
	    wbuf[1] = val;
	    wbuf[2] = 0x22;
	    wbuf[3] = 0x33;
	    wbuf[4] = 0x44;
	    wbuf[5] = 0x55;

	    if(write(fd,wbuf,6) != 6)
		  sys_error("write wbuf failed\n");
	    printf("write data success\n");
      }
      close(fd);
      return 0;
}

