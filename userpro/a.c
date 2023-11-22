#include "stdio.h"
#include "unistd.h"
int _start(int argc,void *argv)
{
  unsigned int count = 0;
  while(1){
	  printf("%d +++++++++++++++user application run! count=%d\n", argc, count++);
	  sleep(1);
  };
  return 5;
}
