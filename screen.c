#include "screen.h"
#include "boot.h"
#include "memcachectl.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define FONT_START_MEM   0x57000
#define FONT_FILE_SIZE   0x1D000
// static unsigned char table_rgb[16 * 3] = {
//     0x00, 0x00, 0x00, /* 0:黑 */
//     0xff, 0x00, 0x00, /* 1:亮红 */
//     0x00, 0xff, 0x00, /* 2:亮绿 */
//     0xff, 0xff, 0x00, /* 3:亮黄 */
//     0x00, 0x00, 0xff, /* 4:亮蓝 */
//     0xff, 0x00, 0xff, /* 5:亮紫 */
//     0x00, 0xff, 0xff, /* 6:浅亮蓝 */
//     0xff, 0xff, 0xff, /* 7:白 */
//     0xc6, 0xc6, 0xc6, /* 8:亮灰 */
//     0x84, 0x00, 0x00, /* 9:暗红 */
//     0x00, 0x84, 0x00, /* 10:暗绿 */
//     0x84, 0x84, 0x00, /* 11:暗黄 */
//     0x00, 0x00, 0x84, /* 12:暗青 */
//     0x84, 0x00, 0x84, /* 13:暗紫 */
//     0x00, 0x84, 0x84, /* 14:浅暗蓝 */
//     0x84, 0x84, 0x84  /* 15:暗灰 */
// };
// void set_palette(int start, int end, unsigned char *rgb)
// {
//     int i;             
//     sysOutChar(0x03c8, start);
//     for (i = start; i <= end; i++)
//     {
//         sysOutChar(0x03c9, rgb[0]>>2);
//         sysOutChar(0x03c9, rgb[1]>>2);
//         sysOutChar(0x03c9, rgb[2]>>2);
//         rgb += 3;
//     }
//     return;
// }

void initScreen()
{
    for (uint32 size = 0; size < 0x10000; size += 0x1000)
    {
        mem4k_map(0xa0000 + size, 0xa0000 + size, MEM_UC, PAGE_RW);
    }
    mem_fix_type_set(0xa0000, 0x10000, MEM_UC);
    memset_s(0xa0000, 15, 0x10000);
}


void fontInit()
{
    for (uint32 size = 0; size < FONT_FILE_SIZE; size += 0x1000)
    {
        mem4k_map(FONT_START_MEM + size, FONT_START_MEM + size, MEM_UC, PAGE_RW);
    }
    mem_fix_type_set(FONT_START_MEM, FONT_FILE_SIZE, MEM_UC);  
    /* 初始化字体 */
    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, FONT_START_MEM, 0))
    {
        printf("stb init font failed\r\n");
    }

}