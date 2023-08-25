#ifndef  SYSLIB_H_H_
#define	 SYSLIB_H_H_


extern void  puts(const char* str);
extern int  readSectors(char* des, int startSector, int count);
extern void  clearScreen();
extern void  exit(unsigned int retval);
extern void  setcursor(unsigned int pos);


#endif // ! SYSLIB_H_H_
