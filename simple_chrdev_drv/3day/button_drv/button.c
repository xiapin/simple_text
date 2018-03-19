#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>

#define LED_ALL_ON _IO('L', 0x1111)
#define LED_ALL_OFF _IO('L', 0x2222)
#define LED_NUM_ON _IOW('L', 0x1234, int) 
#define LED_NUM_OFF _IOW('L', 0x6789, int)

struct button_event{
      int code;
      int status;
};

int main(void)
{
      struct button_event bt_event;

      int button_fd = -1,ret = -1;
      int led_fd;

      button_fd = open("/dev/button",O_RDWR);
      led_fd = open("/dev/led2",O_RDWR);

      if(button_fd < 0 || led_fd < 0){
	    perror("open button_fd or led_fd failed\n");
	    exit(1);
      }
      
      while(1)
      {
	    ret = read(button_fd,&bt_event,sizeof(bt_event));
	    if(ret < 0){
		  perror("read button_fd failed\n");
		  exit(1);
	    }
	    if(bt_event.code == KEY_UP){
		  if(bt_event.status){//按下上键
			printf("usr-->上键--按下\n");

			ioctl(led_fd,LED_ALL_ON);
		  }else{
			printf("usr-->上键--松开\n");
			ioctl(led_fd,LED_ALL_OFF);
		  }	  
	    }else
		  printf("usr-->undown key\n");
      }

      close(button_fd);
      return 0;
}
