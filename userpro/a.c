#include "stdio.h"
#include "unistd.h"
int _start(int argc,void *argv)
{
  unsigned int count = 0xfffff;
  while(1){
	  printf("%d +++++++++++++++user application run!\n", argc);
	  sleep(1);
  };
  return 5;
}
