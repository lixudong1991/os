#include "stdio.h"
#include "stdlib.h"
#include <sys/mman.h>
int _start(int argc,void *argv)
{
  unsigned int count = 0;
  size_t a =  mmap(0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006);
  while(1){
	  printf("%d +++++++++++++++user application run! count=%d %d\n", argc, count++, a);
	  sleep(1);
  };
  return 5;
}
