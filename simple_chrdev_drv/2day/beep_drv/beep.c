#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
      int fd = -1;
      fd = open("/dev/BEEP",O_RDWR);
      if(fd < 0){
	    perror("open failed\n");
	    exit(1);
      }

      int on =1;
      while(1)
      {
	    on = 1;
	    write(fd,&on,sizeof(on));
	    sleep(1);
      }

      return 0;
}
