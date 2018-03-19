#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define LED_ALL_ON _IO('L', 0x1111)
#define LED_ALL_OFF _IO('L', 0x2222)
#define LED_NUM_ON _IOW('L', 0x1234, int)
#define LED_NUM_OFF _IOW('L', 0x6789, int)

int main(void)
{
      int fd = -1;

      fd = open("/dev/led2",O_RDWR);
      if(fd < 0){
	    perror("open failed");
	    exit(1);
      }

#if 0
      int on;
      while(1)
      {
	    on = 1;
	    write(fd,&on,sizeof(on));
	    sleep(1);
	    on = 0;
	    write(fd,&on,sizeof(on));
	    sleep(1);
      }
#else
      while(1)
      {
	    ioctl(fd,LED_NUM_ON,1);
	    sleep(1);
	    ioctl(fd,LED_NUM_OFF,1);
	    sleep(1);

	    ioctl(fd,LED_NUM_ON,2);
	    sleep(1);
	    ioctl(fd,LED_NUM_OFF,2);
	    sleep(1);

	    ioctl(fd,LED_ALL_ON);
	    sleep(1);
	    ioctl(fd,LED_ALL_OFF);
	    sleep(1);
      }
#endif
      close(fd);
      return 0;
}
