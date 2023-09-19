#include "ps2device.h"
#include "osdataPhyAddr.h"
#include "boot.h"
#include "printf.h"
extern uint32 ps2Controllerinit();

enum VariousKeyIndex
{
    CAPSLOCK_INDEX = 0,
    SCROLLLOCK_INDEX,
    NUMBERLOCK_INDEX,
    SHIFT_INDEX,
    ALT_INDEX,
    CONTROL_INDEX,
    ENTER_PRESS
};

#pragma pack(1)
typedef struct KeyBoardStruct
{
    uint8_t ps2ScanCodeBuff[8];
    uint32_t ps2ScanIndex;
    uint32_t ps2KeyCodeBuffIndex;
    uint32_t ps2KeyCodeBuffRec;
    uint32_t Various[10]; // CapsLock, ScrollLock, NumberLock,shift, alt, control
    uint32_t getcharIndex;
    uint8_t ps2KeyCodeBuff[KEY_BUFF_SIZE - 64];
} KeyBoardStruct;
#pragma pack()

KeyBoardStruct *pkeyBoardStruct = KEY_BUFF_ADDR;
uint16_t *scanSet2Map;
uint32 ps2DeviceInit()
{
    uint32 ret = ps2Controllerinit();
    memset_s(pkeyBoardStruct, 0, sizeof(KeyBoardStruct));
    pkeyBoardStruct->Various[ENTER_PRESS] = 1;
    scanSet2Map = kernel_malloc(320);
    memset_s(scanSet2Map, 0, 320);

    scanSet2Map[0x5A] = '\n'; //	enter pressed
    scanSet2Map[0x0D] = 0x9;  // tab pressed
    scanSet2Map[0x0E] = '`';  //(back tick) pressed
    scanSet2Map[0x15] = 'q';  // pressed
    scanSet2Map[0x1A] = 'z';  // pressed
    scanSet2Map[0x1C] = 'a';  // pressed
    scanSet2Map[0x21] = 'c';  // pressed
    scanSet2Map[0x24] = 'e';  // pressed
    scanSet2Map[0x29] = ' ';  // pressed
    scanSet2Map[0x2C] = 't';  // pressed
    scanSet2Map[0x31] = 'n';  // pressed
    scanSet2Map[0x34] = 'g';  // pressed
    scanSet2Map[0x3A] = 'm';  // pressed
    scanSet2Map[0x3C] = 'u';  // pressed
    scanSet2Map[0x41] = ',';  // pressed
    scanSet2Map[0x44] = 'o';  // pressed
    scanSet2Map[0x49] = '.';  // pressed
    scanSet2Map[0x4C] = ';';  // pressed
    scanSet2Map[0x52] = '\''; // pressed
    scanSet2Map[0x54] = '[';  // pressed
    scanSet2Map[0x5B] = ']';  // pressed
    scanSet2Map[0x5D] = '\\'; // pressed
    scanSet2Map[0x55] = '=';  // pressed
    scanSet2Map[0x4E] = '-';  // pressed
    scanSet2Map[0x4D] = 'p';  // pressed
    scanSet2Map[0x4A] = '/';  // pressed
    scanSet2Map[0x4B] = 'l';  // pressed
    scanSet2Map[0x45] = '0';  //(zero) pressed
    scanSet2Map[0x46] = '9';  // pressed
    scanSet2Map[0x42] = 'k';  // pressed
    scanSet2Map[0x43] = 'i';  // pressed
    scanSet2Map[0x3E] = '8';  // pressed
    scanSet2Map[0x36] = '6';  // pressed
    scanSet2Map[0x3D] = '7';  // pressed
    scanSet2Map[0x3B] = 'j';  // pressed
    scanSet2Map[0x35] = 'y';  // pressed
    scanSet2Map[0x32] = 'b';  // pressed
    scanSet2Map[0x33] = 'h';  // pressed
    scanSet2Map[0x2D] = 'r';  // pressed
    scanSet2Map[0x2E] = '5';  // pressed
    scanSet2Map[0x2A] = 'v';  // pressed
    scanSet2Map[0x2B] = 'f';  // pressed
    scanSet2Map[0x25] = '4';  // pressed
    scanSet2Map[0x26] = '3';  // pressed
    scanSet2Map[0x16] = '1';  // pressed
    scanSet2Map[0x1B] = 's';  // pressed
    scanSet2Map[0x1D] = 'w';  // pressed
    scanSet2Map[0x1E] = '2';  // pressed
    scanSet2Map[0x22] = 'x';  // pressed
    scanSet2Map[0x23] = 'd';  // pressed
    scanSet2Map[0x69] = '1';  //(keypad)  pressed
    scanSet2Map[0x6C] = '7';  //(keypad)  pressed
    scanSet2Map[0x70] = '0';  //(keypad)  pressed
    scanSet2Map[0x72] = '2';  //(keypad)  pressed
    scanSet2Map[0x74] = '6';  //(keypad)  pressed
    scanSet2Map[0x7D] = '9';  //(keypad)  pressed
    scanSet2Map[0x79] = '+';  //(keypad)  pressed
    scanSet2Map[0x6B] = '4';  //(keypad)  pressed
    scanSet2Map[0x7A] = '3';  //(keypad)  pressed
    scanSet2Map[0x71] = '.';  //(keypad)  pressed
    scanSet2Map[0x73] = '5';  //(keypad)  pressed
    scanSet2Map[0x75] = '8';  //(keypad)  pressed
    scanSet2Map[0x7C] = '*';  //(keypad)  pressed
    scanSet2Map[0x7B] = '-';  //(keypad)  pressed

    scanSet2Map[0x5A] |= ((uint16_t)'\n') << 8; //	enter pressed
    scanSet2Map[0x0D] |= ((uint16_t)0x9) << 8;  // tab pressed
    scanSet2Map[0x15] |= ((uint16_t)'Q') << 8;  // pressed
    scanSet2Map[0x1A] |= ((uint16_t)'Z') << 8;  // pressed
    scanSet2Map[0x1C] |= ((uint16_t)'A') << 8;  // pressed
    scanSet2Map[0x21] |= ((uint16_t)'C') << 8;  // pressed
    scanSet2Map[0x24] |= ((uint16_t)'E') << 8;  // pressed
    scanSet2Map[0x29] |= ((uint16_t)' ') << 8;  // pressed
    scanSet2Map[0x2C] |= ((uint16_t)'T') << 8;  // pressed
    scanSet2Map[0x31] |= ((uint16_t)'N') << 8;  // pressed
    scanSet2Map[0x34] |= ((uint16_t)'G') << 8;  // pressed
    scanSet2Map[0x3A] |= ((uint16_t)'M') << 8;  // pressed
    scanSet2Map[0x3C] |= ((uint16_t)'U') << 8;  // pressed
    scanSet2Map[0x44] |= ((uint16_t)'O') << 8;  // pressed
    scanSet2Map[0x4D] |= ((uint16_t)'P') << 8;  // pressed
    scanSet2Map[0x4B] |= ((uint16_t)'L') << 8;  // pressed
    scanSet2Map[0x42] |= ((uint16_t)'K') << 8;  // pressed
    scanSet2Map[0x43] |= ((uint16_t)'I') << 8;  // pressed
    scanSet2Map[0x3B] |= ((uint16_t)'J') << 8;  // pressed
    scanSet2Map[0x35] |= ((uint16_t)'Y') << 8;  // pressed
    scanSet2Map[0x32] |= ((uint16_t)'B') << 8;  // pressed
    scanSet2Map[0x33] |= ((uint16_t)'H') << 8;  // pressed
    scanSet2Map[0x2D] |= ((uint16_t)'R') << 8;  // pressed
    scanSet2Map[0x2A] |= ((uint16_t)'V') << 8;  // pressed
    scanSet2Map[0x2B] |= ((uint16_t)'F') << 8;  // pressed
    scanSet2Map[0x1B] |= ((uint16_t)'S') << 8;  // pressed
    scanSet2Map[0x1D] |= ((uint16_t)'W') << 8;  // pressed
    scanSet2Map[0x22] |= ((uint16_t)'X') << 8;  // pressed
    scanSet2Map[0x23] |= ((uint16_t)'D') << 8;  // pressed
    scanSet2Map[0x69] |= ((uint16_t)'1') << 8;  //(keypad)  pressed
    scanSet2Map[0x6C] |= ((uint16_t)'7') << 8;  //(keypad)  pressed
    scanSet2Map[0x70] |= ((uint16_t)'0') << 8;  //(keypad)  pressed
    scanSet2Map[0x72] |= ((uint16_t)'2') << 8;  //(keypad)  pressed
    scanSet2Map[0x74] |= ((uint16_t)'6') << 8;  //(keypad)  pressed
    scanSet2Map[0x7D] |= ((uint16_t)'9') << 8;  //(keypad)  pressed
    scanSet2Map[0x79] |= ((uint16_t)'+') << 8;  //(keypad)  pressed
    scanSet2Map[0x6B] |= ((uint16_t)'4') << 8;  //(keypad)  pressed
    scanSet2Map[0x7A] |= ((uint16_t)'3') << 8;  //(keypad)  pressed
    scanSet2Map[0x71] |= ((uint16_t)'.') << 8;  //(keypad)  pressed
    scanSet2Map[0x73] |= ((uint16_t)'5') << 8;  //(keypad)  pressed
    scanSet2Map[0x75] |= ((uint16_t)'8') << 8;  //(keypad)  pressed
    scanSet2Map[0x7C] |= ((uint16_t)'*') << 8;  //(keypad)  pressed
    scanSet2Map[0x7B] |= ((uint16_t)'-') << 8;  //(keypad)  pressed
    scanSet2Map[0x0E] |= ((uint16_t)'~') << 8;  //(back tick) pressed
    scanSet2Map[0x16] |= ((uint16_t)'!') << 8;  // pressed
    scanSet2Map[0x1E] |= ((uint16_t)'@') << 8;  // pressed
    scanSet2Map[0x26] |= ((uint16_t)'#') << 8;  // pressed
    scanSet2Map[0x25] |= ((uint16_t)'$') << 8;  // pressed
    scanSet2Map[0x2E] |= ((uint16_t)'%') << 8;  // pressed
    scanSet2Map[0x36] |= ((uint16_t)'^') << 8;  // pressed
    scanSet2Map[0x3D] |= ((uint16_t)'&') << 8;  // pressed
    scanSet2Map[0x3E] |= ((uint16_t)'*') << 8;  // pressed
    scanSet2Map[0x46] |= ((uint16_t)'(') << 8;  // pressed
    scanSet2Map[0x45] |= ((uint16_t)')') << 8;  //(zero) pressed
    scanSet2Map[0x4E] |= ((uint16_t)'_') << 8;  // pressed
    scanSet2Map[0x55] |= ((uint16_t)'+') << 8;  // pressed
    scanSet2Map[0x54] |= ((uint16_t)'{') << 8;  // pressed
    scanSet2Map[0x5B] |= ((uint16_t)'}') << 8;  // pressed
    scanSet2Map[0x5D] |= ((uint16_t)'|') << 8;  // pressed
    scanSet2Map[0x4C] |= ((uint16_t)':') << 8;  // pressed
    scanSet2Map[0x52] |= ((uint16_t)'"') << 8;  // pressed
    scanSet2Map[0x41] |= ((uint16_t)'<') << 8;  // pressed
    scanSet2Map[0x49] |= ((uint16_t)'>') << 8;  // pressed
    scanSet2Map[0x4A] |= ((uint16_t)'?') << 8;  // pressed

    return ret;
}

void ps2KeyInterruptProc(uint32_t code)
{
    pkeyBoardStruct->ps2ScanCodeBuff[pkeyBoardStruct->ps2ScanIndex] = (uint8_t)code;
    if (pkeyBoardStruct->ps2ScanIndex == 0)
    {
        if (code == 0xf0 || code == 0xe0)
        {
            pkeyBoardStruct->ps2ScanIndex++;
        }
        else // pressed
        {
            char c = (char)(scanSet2Map[code]);
            if (c == 0)
            {
                switch (code)
                {
                case 0x58: //	CapsLock pressed
                {
                    if (pkeyBoardStruct->Various[CAPSLOCK_INDEX])
                        pkeyBoardStruct->Various[CAPSLOCK_INDEX] = 0;
                    else
                        pkeyBoardStruct->Various[CAPSLOCK_INDEX] = 1;
                }
                break;
                case 0x7E: //	ScrollLock pressed
                {
                }
                break;
                case 0x12: //	left shift pressed
                case 0x59: // right shift pressed
                {
                    pkeyBoardStruct->Various[SHIFT_INDEX] = 1;
                }
                break;
                case 0x77: //	NumberLock pressed
                {
                }
                break;
                default:
                    break;
                }
            }
            else
            {
                if (pkeyBoardStruct->ps2KeyCodeBuffIndex == KEY_BUFF_SIZE - 64)
                    pkeyBoardStruct->ps2KeyCodeBuffIndex = 0;
                if (pkeyBoardStruct->Various[ENTER_PRESS])
                    return;
                if (c >= 'a' && c <= 'z')
                {
                    if (pkeyBoardStruct->Various[CAPSLOCK_INDEX])
                    {
                        if (pkeyBoardStruct->Various[SHIFT_INDEX])
                            pkeyBoardStruct->ps2KeyCodeBuff[pkeyBoardStruct->ps2KeyCodeBuffIndex] = c;
                        else
                            pkeyBoardStruct->ps2KeyCodeBuff[pkeyBoardStruct->ps2KeyCodeBuffIndex] = (uint8_t)(scanSet2Map[code] >> 8);
                    }
                    else if (pkeyBoardStruct->Various[SHIFT_INDEX])
                    {
                        pkeyBoardStruct->ps2KeyCodeBuff[pkeyBoardStruct->ps2KeyCodeBuffIndex] = (uint8_t)(scanSet2Map[code] >> 8);
                    }
                    else
                        pkeyBoardStruct->ps2KeyCodeBuff[pkeyBoardStruct->ps2KeyCodeBuffIndex] = c;
                }
                else
                {
                    if (c == '\n')
                        pkeyBoardStruct->Various[ENTER_PRESS] = 1;
                    if (pkeyBoardStruct->Various[SHIFT_INDEX])
                    {
                        pkeyBoardStruct->ps2KeyCodeBuff[pkeyBoardStruct->ps2KeyCodeBuffIndex] = (uint8_t)(scanSet2Map[code] >> 8);
                    }
                    else
                        pkeyBoardStruct->ps2KeyCodeBuff[pkeyBoardStruct->ps2KeyCodeBuffIndex] = c;
                }
                putchar(pkeyBoardStruct->ps2KeyCodeBuff[pkeyBoardStruct->ps2KeyCodeBuffIndex]);
                pkeyBoardStruct->ps2KeyCodeBuffIndex++;
            }
        }
    }
    else if (pkeyBoardStruct->ps2ScanIndex == 1)
    {
        uint16_t scancode = *(uint16 *)(pkeyBoardStruct->ps2ScanCodeBuff);
        switch (scancode)
        {
        case 0xf0e0:
            pkeyBoardStruct->ps2ScanIndex++;
            break;
        case 0x5ae0:
        {
            if (pkeyBoardStruct->ps2KeyCodeBuffIndex == KEY_BUFF_SIZE - 64)
                pkeyBoardStruct->ps2KeyCodeBuffIndex = 0;
            pkeyBoardStruct->ps2KeyCodeBuff[pkeyBoardStruct->ps2KeyCodeBuffIndex] = '\n';
            putchar(pkeyBoardStruct->ps2KeyCodeBuff[pkeyBoardStruct->ps2KeyCodeBuffIndex]);
            pkeyBoardStruct->ps2KeyCodeBuffIndex++;
            pkeyBoardStruct->ps2ScanIndex = 0;
        }
        break;
        case 0x12f0:
        case 0x59f0:
        {
            pkeyBoardStruct->Various[SHIFT_INDEX] = 0;
            pkeyBoardStruct->ps2ScanIndex = 0;
        }
        break;
        case 0x4ae0:
        {
            if (pkeyBoardStruct->ps2KeyCodeBuffIndex == KEY_BUFF_SIZE - 64)
                pkeyBoardStruct->ps2KeyCodeBuffIndex = 0;
            pkeyBoardStruct->ps2KeyCodeBuff[pkeyBoardStruct->ps2KeyCodeBuffIndex] = '/';
            putchar(pkeyBoardStruct->ps2KeyCodeBuff[pkeyBoardStruct->ps2KeyCodeBuffIndex]);
            pkeyBoardStruct->ps2KeyCodeBuffIndex++;
            pkeyBoardStruct->ps2ScanIndex = 0;
        }
        break;
        default:
            pkeyBoardStruct->ps2ScanIndex = 0;
            break;
        }
    }
    else
    {
        pkeyBoardStruct->ps2ScanIndex = 0;
    }
}

uint32 getchar()
{
    while (1)
    {
        if (pkeyBoardStruct->Various[ENTER_PRESS])
        {
            if (pkeyBoardStruct->ps2KeyCodeBuffIndex != 0)
            {
                if (pkeyBoardStruct->getcharIndex < pkeyBoardStruct->ps2KeyCodeBuffIndex)
                {
                    return pkeyBoardStruct->ps2KeyCodeBuff[pkeyBoardStruct->getcharIndex++];
                }
                else
                {
                    pkeyBoardStruct->getcharIndex = 0;
                    pkeyBoardStruct->ps2KeyCodeBuffIndex = 0;
                    pkeyBoardStruct->Various[ENTER_PRESS] = 0;
                }
            }
            else
            {
                pkeyBoardStruct->getcharIndex = 0;
                pkeyBoardStruct->Various[ENTER_PRESS] = 0;
            }
               
        }
            
        asm("sti");
        asm("hlt");
    }

}
int fgets(char* s, int size)
{
    char c = 0;
    int len = 0;
    do
    {
        c =(char)getchar();
        s[len] = c;
        len++;
    } while (c != '\n'&& len<(size-1));
    s[len] = 0;
    return len;
}