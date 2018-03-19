#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
      int fd = -1;

      fd = open("/dev/hello2",O_RDWR);
      if(fd < 0){
	    perror("open failed");
	    exit(1);
      }
      return 0;
}
