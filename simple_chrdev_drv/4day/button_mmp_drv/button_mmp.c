#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <linux/input.h>
#include <sys/mman.h>

#define BUTTON_IOC_DATA 0x1122
#define _AC(x,y) x
#define PAGE_SHIFT 12
#define PAGE_SIZE (_AC(1,UL) << PAGE_SHIFT)
#define LED_ALL_ON _IO('L', 0x1111)
#define LED_ALL_OFF _IO('L', 0x2222)
#define LED_NUM_ON _IOW('L', 0x1234, int) 
#define LED_NUM_OFF _IOW('L', 0x6789, int)

struct button_event{
      int code;
      int status;
};

struct mem_data{
      char buf[128];
};

int main(void)
{
      int on = 1,off = 0;
      struct button_event bt_event;

      int button_fd = -1,ret = -1;
      int led_fd;
      int beep_fd;

      beep_fd = open("/dev/BEEP",O_RDWR);
      button_fd = open("/dev/button",O_RDWR);
      led_fd = open("/dev/led2",O_RDWR);

      if(button_fd < 0 || led_fd < 0){
	    perror("open button_fd or led_fd failed\n");
	    exit(1);
      }
      /*测试mmp功能*/
      struct mem_data data;
      char str[128];
      char *addr = (char *)mmap(NULL,PAGE_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED
		  ,button_fd,0);
      if(addr = NULL){
	    perror("mmp error\n");
	    exit(1);
      }

      fgets(str,128,0);
      memcpy(addr,str,strlen(str));
      sleep(1);

      ret = ioctl(button_fd,BUTTON_IOC_DATA,&data);
      if(ret < 0){
	    perror("IOCtl error\n");
	    exit(1);
      }
      /*利用pool同时监控标准输入及键盘按键*/
      struct pollfd pfds[2];
      char buf[128];

      pfds[0].fd = STDIN_FILENO;/*标准输入*/
      pfds[0].events = POLLIN;  /*是否可读*/

      pfds[1].fd = button_fd;/*按键输入*/
      pfds[1].events = POLLIN;

      while(1)
      {
	    ret = poll(pfds,2,-1);
	    if(ret < 0){
		  perror("poll error\n");
		  exit(1);
	    }
	    if(ret > 0)
	    {
		  if(pfds[0].revents & POLLIN){
			bzero(buf,128);
			fgets(buf,128,stdin);
			printf("buf:%s",buf);
		  }

		  if(pfds[1].revents & POLLIN)
		  {
			memset(&bt_event,0,sizeof(bt_event));
			ret = read(button_fd,&bt_event,sizeof(bt_event));
			if(ret < 0){
			      perror("read button_fd failed\n");
			      exit(1);
			}
#if 0 
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
#else 
			switch(bt_event.code)
			{
			      case KEY_UP:
				    if(bt_event.status){/*按下*/
					  printf("usr:key_up down\n");
				    }else{
					  printf("usr:key_up undown\n");
				    }
				    break;
			      case KEY_DOWN:
				    if(bt_event.status){/*按下*/
					  printf("usr:key_down down\n");
				    }else{
					  printf("usr:key_down undown\n");
				    }
				    break;
			      case KEY_LEFT:
				    if(bt_event.status){/*按下*/
					  printf("usr:key_left down\n");
				    }else{
					  printf("usr:key_left undown\n");
				    }
				    break;
			      case KEY_RIGHT:
				    if(bt_event.status){/*按下*/
					  printf("usr:key_right down\n");
				    }else{
					  printf("usr:key_right undown\n");
				    }
				    break;
			      default:
				    printf("unknown key\n");
				    break;
			}

#endif
		  }
	    }
      }
      
      close(beep_fd);
      close(led_fd);
      close(button_fd);
      return 0;
}
