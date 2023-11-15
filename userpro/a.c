#include "stdio.h"

int _start(int argc,void *argv)
{
  unsigned int count = 0xfffff;
  while(1){
	  printf("%d +++++++++++++++user application run!\n", argc);
	  count = 0xfffffff;
	  while (count--) {}
  };
  return 5;
}
