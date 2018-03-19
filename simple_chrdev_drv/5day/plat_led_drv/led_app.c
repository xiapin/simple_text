#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int main(void)
{
      int fd =open("/dev/plat_led",O_RDWR);
      if(fd < 0 ){
	    perror("open failed\n");
	    exit(1);
      }

      int on = 1;
      int off = 0;
      while(1)
      {
	    write(fd,&on,sizeof(on));
	    sleep(1);
	    
	    write(fd,&off,sizeof(off));
	    sleep(1);
      }
      close(fd);
      return 0;
}
