#include "screen.h"
#include "boot.h"
#include "memcachectl.h"
#include "printf.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define FONT_START_MEM 0x57000
#define FONT_FILE_SIZE 0x1D000
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
    memset_s(0xa0000, 15, 320*200);
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
        printf("stb init font failed\n");
    }
    else
        printf("stb init font success\n");

    char word[] ="STBfont";    
    /* 创建位图 */
    int bitmap_w = 320; /* 位图的宽 */
    int bitmap_h = 200; /* 位图的高 */
    char *bitmap = kernel_malloc(bitmap_w * bitmap_h);
    memset_s(bitmap,0,320*200);
    /* 计算字体缩放 */
    float pixels = 12.0;                                    /* 字体大小（字号） */
    float scale = stbtt_ScaleForPixelHeight(&info, pixels); /* scale = pixels / (ascent - descent) */

    /**
     * 获取垂直方向上的度量
     * ascent：字体从基线到顶部的高度；
     * descent：基线到底部的高度，通常为负值；
     * lineGap：两个字体之间的间距；
     * 行间距为：ascent - descent + lineGap。
     */
    int ascent = 0;
    int descent = 0;
    int lineGap = 0;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    /* 根据缩放调整字高 */
    ascent = roundf(ascent * scale);
    descent = roundf(descent * scale);

    int x = 0; /*位图的x*/

 /* 循环加载word中每个字符 */
    for (int i = 0; i < 7; ++i)
    {
        /** 
          * 获取水平方向上的度量
          * advanceWidth：字宽；
          * leftSideBearing：左侧位置；
        */
        int advanceWidth = 0;
        int leftSideBearing = 0;
        stbtt_GetCodepointHMetrics(&info, word[i], &advanceWidth, &leftSideBearing);

        /* 获取字符的边框（边界） */
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

        /* 计算位图的y (不同字符的高度不同） */
        int y = ascent + c_y1;

        /* 渲染字符 */
        int byteOffset = x + roundf(leftSideBearing * scale) + (y * bitmap_w);
        stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, bitmap_w, scale, scale, word[i]);

        /* 调整x */
        x += roundf(advanceWidth * scale);

        /* 调整字距 */
        int kern;
        kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
        x += roundf(kern * scale);
    }

    memcpy_s(0xa0000,bitmap,320*200);
}