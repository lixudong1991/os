#include "ps2device.h"
#include "osdataPhyAddr.h"
#include "boot.h"
extern uint32 ps2Controllerinit();

#define CAPSLOCK_INDEX 0
#define SCROLLLOCK_INDEX 0
#define NUMBERLOCK_INDEX 0
#define SHIFT_INDEX 0
#define ALT_INDEX 0
#define CONTROL_INDEX 0

#pragma pack(1)
typedef struct KeyBoardStruct
{
    uint8_t  ps2ScanCodeBuff[8];
    uint32_t ps2ScanIndex;
    uint32_t ps2KeyCodeBuffIndex;
    uint32_t ps2KeyCodeBuffRec;
    uint32_t Various[11];//CapsLock, ScrollLock, NumberLock,shift, alt, control
    uint8_t  ps2KeyCodeBuff[KEY_BUFF_SIZE-64];
}KeyBoardStruct;
#pragma pack()

KeyBoardStruct *pkeyBoardStruct =KEY_BUFF_ADDR;
uint8_t  *scanSet2Map;

uint32 ps2DeviceInit()
{
    uint32 ret = ps2Controllerinit();
    memset_s(pkeyBoardStruct,0,sizeof(KeyBoardStruct));
    scanSet2Map = kernel_malloc(160);
    memset_s(scanSet2Map,0,160);
    scanSet2Map[0x0E]='`';//(back tick) pressed	
scanSet2Map[0x15]='Q';//pressed			
scanSet2Map[0x1A]='Z';//pressed	
scanSet2Map[0x1C]='A';//pressed				
scanSet2Map[0x21]='C';//pressed		
scanSet2Map[0x24]='E';//pressed				
scanSet2Map[0x29]=' ';//pressed		
scanSet2Map[0x2C]='T';//pressed				
scanSet2Map[0x31]='N';//pressed		
scanSet2Map[0x34]='G';//pressed				
scanSet2Map[0x3A]='M';//pressed	
scanSet2Map[0x3C]='U';//pressed			
scanSet2Map[0x41]=',';//pressed		
scanSet2Map[0x44]='O';//pressed				
scanSet2Map[0x49]='.';//pressed		
scanSet2Map[0x4C]=';';//pressed				
scanSet2Map[0x52]='\''; //pressed		
scanSet2Map[0x54]='[';//pressed					
scanSet2Map[0x5B]=']';//pressed
scanSet2Map[0x5D]='\\';//pressed
scanSet2Map[0x55]='=';//pressed
scanSet2Map[0x4E]='-';//pressed
scanSet2Map[0x4D]='P';//pressed
scanSet2Map[0x4A]='/';//pressed
scanSet2Map[0x4B]='L';//pressed
scanSet2Map[0x45]='0';//(zero) pressed
scanSet2Map[0x46]='9';//pressed
scanSet2Map[0x42]='K';//pressed
scanSet2Map[0x43]='I';//pressed
scanSet2Map[0x3E]='8';//pressed	
scanSet2Map[0x36]='6';//pressed
scanSet2Map[0x3D]='7';//pressed
scanSet2Map[0x3B]='J';//pressed
scanSet2Map[0x35]='Y';//pressed
scanSet2Map[0x32]='B';//pressed
scanSet2Map[0x33]='H';//pressed
scanSet2Map[0x2D]='R';//pressed
scanSet2Map[0x2E]='5';//pressed
scanSet2Map[0x2A]='V';//pressed
scanSet2Map[0x2B]='F';//pressed
scanSet2Map[0x25]='4';//pressed
scanSet2Map[0x26]='3';//pressed
scanSet2Map[0x16]='1';//pressed
scanSet2Map[0x1B]='S';//pressed
scanSet2Map[0x1D]='W';//pressed
scanSet2Map[0x1E]='2';//pressed
scanSet2Map[0x22]='X';//pressed

scanSet2Map[0x23]='D';//pressed
    return ret;
}


void ps2KeyInterruptProc(uint32_t code)
{
    pkeyBoardStruct->ps2ScanCodeBuff[pkeyBoardStruct->ps2ScanIndex]=(uint8_t)code;
    if(pkeyBoardStruct->ps2ScanIndex == 0)
    {
        if(code!=0xf0&&code!=0xe0)
        {
            pkeyBoardStruct->ps2ScanIndex++;
        }
        else //pressed
        {
            
        }   
    }
    else if(pkeyBoardStruct->ps2ScanIndex == 1)
    {
        if(code == 0xf0)
        {
            pkeyBoardStruct->ps2ScanIndex++;
        }
        else  //release
        {
            pkeyBoardStruct->ps2ScanIndex = 0;
           
        }
    }
    else
    {
        pkeyBoardStruct->ps2ScanIndex = 0;     
    }
}

