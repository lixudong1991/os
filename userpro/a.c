#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
int _start(int argc,void *argv)
{
  unsigned int count = 0;
  char* p = malloc(0x10000);
  free(p);
  while(1){
	  printf("%d +++++++++++++++user application run! count=%d\n", argc, count++);
	  sleep(1);
  };
  return 5;
}
