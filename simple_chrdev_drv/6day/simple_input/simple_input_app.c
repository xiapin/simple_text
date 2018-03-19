
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>
#include <poll.h>

int main(int argc, char *argv[])
{
      int ret;
      struct input_event event;

      int fd = open("/dev/event0", O_RDWR);
      if(fd < 0)
      {
	    perror("open");
	    exit(1);
      }


      while(1)
      {
        memset(&event,0,sizeof(event));
        
	    ret = read(fd, &event, sizeof(struct input_event));
	    if(ret < 0)
	    {
		  perror("read error\n");
		  exit(1);
	    }

	    if(event.type == EV_KEY)
	    {
		  if(event.code == KEY_UP)
		  {
			if(event.value)
			      printf("<APP>-------KEY_UP pressed\n");
			else
			      printf("<APP>-------KEY_UP up\n"); 
		  }
          if(event.code == KEY_DOWN)
		  {
			if(event.value)
			      printf("<APP>-------KEY_DOWN pressed\n");
			else
			      printf("<APP>-------KEY_DOWN up\n"); 
		  }
          if(event.code == KEY_LEFT)
		  {
			if(event.value)
			      printf("<APP>-------KEY_LEFT pressed\n");
			else
			      printf("<APP>-------KEY_LEFT up\n"); 
		  }
          if(event.code == KEY_RIGHT)
		  {
			if(event.value)
			      printf("<APP>-------KEY_RIGHT pressed\n");
			else
			      printf("<APP>-------KEY_RIGHT up\n"); 
		  }         
	    }
      }
	  close(fd);
      
      return 0;
}







