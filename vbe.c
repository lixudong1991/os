#include "vbe.h"
#include "stdint.h"
#include "boot.h"
#include "stdio.h"
#include "stdlib.h"
#include "memcachectl.h"
#include "string.h"
#include "ff.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" /* http://nothings.org/stb/stb_image_write.h */
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

char g_vbebuff[VBE_BUFF_SIZE];
char *gfontfilebuff = NULL;
stbtt_fontinfo *g_fontinfo = NULL;
static char *g_DrawTextBuff = NULL;
static Bitmap g_BitmapCache;
#define G_DRAW_TEXTBUFF_W 320
#define G_DRAW_TEXTBUFF_H 240

PMInfoBlock *findPMInfoBlock()
{
    char *ret = NULL;
    // 根据vbe规范，PMInfoBlock位于BIOS image前32kb内存内
    uint32_t startaddr = 0xc0000, endaddr = 0xc8000;
    for (int index = startaddr; index < endaddr; index += 0x1000)
        mem4k_map(index, index, MEM_UC, PAGE_G | PAGE_RW);
    char rsdpsig[] = "PMID";
    int nextbuff[4] = {0};
    int rsdtindex = 0;
    while (startaddr < endaddr)
    {
        rsdtindex = IndexStr_KMP(startaddr, endaddr - startaddr, rsdpsig, nextbuff, 4);
        if (rsdtindex != -1)
        {
            char *temp = (char *)(startaddr + rsdtindex);
            char checksum = 0;
            for (int i = 0; i < sizeof(PMInfoBlock); i++)
            {
                checksum += temp[i];
            }
            if (checksum == 0)
            {
                ret = temp;
                break;
            }
        }
        startaddr += (rsdtindex + 4);
    }
    return (PMInfoBlock *)ret;
}

void initVbe()
{
    PMInfoBlock *ppminfo = findPMInfoBlock();

    // asm("cli");
    char temp = g_vbebuff[4];
    g_vbebuff[4] = 0;
    printf("VBE  signature=%s\n", g_vbebuff);
    g_vbebuff[4] = temp;
    vbe_info_structure *pvbeinfo = g_vbebuff;
    uint32_t vbeAddr = (((pvbeinfo->video_modes >> 16) & 0xffff) << 4) + (pvbeinfo->video_modes & 0xffff);
    mem4k_map(vbeAddr & PAGE_ADDR_MASK, vbeAddr & PAGE_ADDR_MASK, MEM_UC, PAGE_G | PAGE_RW);
    printf("VBE  video_modesAddr=0x%x modes:", vbeAddr);
    uint16_t *pmodelist = g_vbebuff + 768;
    uint32_t modecount = 0;
    uint32_t *pModesResolutionBuff = g_vbebuff + (VBE_BUFF_SIZE - 768);
    while (*pmodelist != 0xffff)
    {
        if (*pmodelist != 0)
        {
            printf("0x%x:0x%x ", *pmodelist, *pModesResolutionBuff);
            modecount++;
        }
        pmodelist++;
        pModesResolutionBuff++;
    }
    pmodelist++;
    char *buffstr = (char *)pmodelist;
    vbe_mode_info_structure *pmodinfo = g_vbebuff + 512;
#if 0
    printf("modecount %d\n", modecount);
    printf("VBE  version=0x%x\n", pvbeinfo->version);
    printf("VBE  oem=0x%x\n", pvbeinfo->oem);
    printf("VBE  capabilities=0x%x\n", pvbeinfo->capabilities);
    printf("VBE  video_memory=0x%x\n", pvbeinfo->video_memory);
    printf("VBE  software_rev=0x%x\n", pvbeinfo->software_rev);
    vbeAddr = (((pvbeinfo->vendor >> 16) & 0xffff) << 4) + (pvbeinfo->vendor & 0xffff);
    printf("VBE  vendor addr=0x%x\n", vbeAddr);
    vbeAddr = (((pvbeinfo->product_name >> 16) & 0xffff) << 4) + (pvbeinfo->product_name & 0xffff);
    printf("VBE  product_name addr=0x%x\n", vbeAddr);
    vbeAddr = (((pvbeinfo->product_rev >> 16) & 0xffff) << 4) + (pvbeinfo->product_rev & 0xffff);
    printf("VBE  product_rev addr=0x%x\n", vbeAddr);
  
    printf("VBE  vendor:product_name:product_rev string=%s\n", buffstr);
    if (ppminfo)
    {
        char pmsigstr[8];
        memcpy_s(pmsigstr, ppminfo->Signature, 4);
        pmsigstr[4] = 0;
        printf("PMInfoBlock sig=%s\n", pmsigstr);
    }
#endif
    printf("VBE  mode  attr:0x%x\n", pmodinfo->ModeAttributes);
    printf("VBE  mode  BytesPerScanLine:0x%x XResolution:0x%x YResolution 0x%x\n", pmodinfo->BytesPerScanLine, pmodinfo->XResolution, pmodinfo->YResolution);
    printf("VBE  mode  XCharSize:0x%x YCharSize 0x%x\n", pmodinfo->XCharSize, pmodinfo->YCharSize);
    printf("VBE  mode  NumberOfPlanes:0x%x BitsPerPixel:0x%x NumberOfBanks:0x%x MemoryModel:0x%x BankSize:0x%x NumberOfImagePages:0x%x\n", pmodinfo->NumberOfPlanes, pmodinfo->BitsPerPixel, pmodinfo->NumberOfBanks, pmodinfo->MemoryModel, pmodinfo->BankSize, pmodinfo->NumberOfImagePages);
    printf("VBE  mode  RedMaskSize:0x%x RedFieldPosition:0x%x GreenMaskSize:0x%x GreenFieldPosition:0x%x BlueMaskSize:0x%x BlueFieldPosition:0x%x DirectColorModeInfo:0x%x PhysBasePtr:0x%x Reserved1:%x Reserved2:%x\n", pmodinfo->RedMaskSize, pmodinfo->RedFieldPosition, pmodinfo->GreenMaskSize, pmodinfo->GreenFieldPosition,
           pmodinfo->BlueMaskSize, pmodinfo->BlueFieldPosition, pmodinfo->DirectColorModeInfo, pmodinfo->PhysBasePtr, pmodinfo->Reserved1, pmodinfo->Reserved2);
    if (pvbeinfo->version >= 0x300)
    {
        printf("VBE3.0 LinBytesPerScanLine:0x%x BnkNumberOfImagePages:0x%x LinNumberOfImagePages 0x%x MaxPixelClock:0x%x\n", pmodinfo->LinBytesPerScanLine, pmodinfo->BnkNumberOfImagePages, pmodinfo->LinNumberOfImagePages, pmodinfo->MaxPixelClock);
        printf("VBE3.0 LinRedMaskSize:0x%x LinRedFieldPosition:0x%x LinGreenMaskSize:0x%x LinGreenFieldPosition:0x%x LinBlueMaskSize:0x%x LinBlueFieldPosition:0x%x\n", pmodinfo->LinRedMaskSize, pmodinfo->LinRedFieldPosition, pmodinfo->LinGreenMaskSize, pmodinfo->LinGreenFieldPosition,
               pmodinfo->LinBlueMaskSize, pmodinfo->LinBlueFieldPosition);
    }

    // asm("sti");
    // sizeof(vbe_mode_info_structure)
    if (pmodinfo->PhysBasePtr)
    {
        uint32_t vbeframbuffsize = pmodinfo->PhysBasePtr + pmodinfo->BytesPerScanLine * pmodinfo->YResolution;
        if (pvbeinfo->version >= 0x300)
            vbeframbuffsize = pmodinfo->PhysBasePtr + pmodinfo->LinBytesPerScanLine * pmodinfo->YResolution;
        vbeframbuffsize -= 1;
        vbeframbuffsize &= PAGE_ADDR_MASK;
        for (uint32_t vbeframbuff = pmodinfo->PhysBasePtr & PAGE_ADDR_MASK; vbeframbuff <= vbeframbuffsize; vbeframbuff += 0x1000)
            mem4k_map(vbeframbuff, vbeframbuff, MEM_UC, PAGE_G | PAGE_RW);
        g_BitmapCache.data =kernel_malloc(pmodinfo->BytesPerScanLine * pmodinfo->YResolution);
    }
     
}

void initFont()
{
    FRESULT res; // 局部变量
    uint32_t br = 0, filesize = 0;
    FIL fp;
    res = f_open(&fp, "/font/ARIALN.ttf", FA_OPEN_ALWAYS | FA_READ);
    printf("open font file res = %d \n", res);

    if (res == FR_OK)
    {
        filesize = f_size(&fp);
        printf("font file size 0x%x \n", filesize);
        gfontfilebuff = kernel_malloc(filesize);
        res = f_read(&fp, gfontfilebuff, filesize, &br);
        if (res == FR_OK)
        {
            printf(" read font data size %x\n", br);
        }
        f_close(&fp);
    }
    // g_fontinfo
    if (!stbtt_InitFont(g_fontinfo, gfontfilebuff, 0))
    {
        printf("stb init font failed\n");
    }
    else
        printf("stb init font success\n");
    g_DrawTextBuff = kernel_malloc(G_DRAW_TEXTBUFF_W * G_DRAW_TEXTBUFF_H); // 最大40*40像素
}
void getScreenPixSize(Pair *size)
{
    vbe_mode_info_structure *pmodinfo = g_vbebuff + 512;
    if (size)
    {
        size->x = pmodinfo->XResolution;
        size->y = pmodinfo->YResolution;
    }
}
int rectIsVaild(Rect *rect)
{
    vbe_mode_info_structure *pmodinfo = g_vbebuff + 512;
    if (rect->right < rect->left || rect->bottom < rect->top)
        return FALSE;
    if (rect->left >= pmodinfo->XResolution || rect->top >= pmodinfo->YResolution)
        return FALSE;
    return TRUE;    
}
void fillRect(Rect *rect, uint32_t fillcolor)
{
    vbe_info_structure *pvbeinfo = g_vbebuff;
    vbe_mode_info_structure *pmodinfo = g_vbebuff + 512;
    if(!rectIsVaild(rect))
        return;
    Rect vaildrect;
    vaildrect.top = rect->top;
    vaildrect.left = rect->left;
    vaildrect.right = (rect->right >= pmodinfo->XResolution ? (pmodinfo->XResolution - 1) : rect->right);
    vaildrect.bottom = (rect->bottom >= pmodinfo->YResolution ? (pmodinfo->YResolution - 1) : rect->bottom);
    if (pmodinfo->BitsPerPixel == 0x10) // 5:6:5
    {
        uint16_t color = 0;
        // r
        uint16_t temp = (uint16_t)((fillcolor >> 16) & 0xff);
        temp = (((temp & 0x1f) << 11));
        color |= temp;
        // g
        temp = (uint16_t)((fillcolor >> 8) & 0xff);
        temp = (((temp & 0x3f) << 5));
        color |= temp;
        // b
        temp = (uint16_t)((fillcolor)&0xff);
        temp = (temp & 0x1f);
        color |= temp;

        uint16_t *pvbeframbuff = pmodinfo->PhysBasePtr;
        uint32_t yindex = vaildrect.top * pmodinfo->XResolution;

        for (uint32_t i = vaildrect.top; i <= vaildrect.bottom; ++i, yindex += pmodinfo->XResolution)
        {
            // for (uint32_t j = rect->left; j <= rect->right; ++j)
            // {
            //     pvbeframbuff[yindex + j] = color;
            // }
            memWordset_s(&(pvbeframbuff[yindex + vaildrect.left]), color, vaildrect.right - vaildrect.left + 1);
        }
    }
    else if (pmodinfo->BitsPerPixel == 0x20) // 8:8:8:8
    {
        uint32_t *pvbeframbuff = pmodinfo->PhysBasePtr;
        uint32_t yindex = rect->top * pmodinfo->XResolution;
        uint32_t color = fillcolor & 0xffffff;
        for (uint32_t i = rect->top; i <= rect->bottom; ++i, yindex += pmodinfo->XResolution)
        {
            // for (uint32_t j = rect->left; j <= rect->right; ++j)
            // {
            //     pvbeframbuff[yindex + j] = color;
            // }
            memDWordset_s(&(pvbeframbuff[yindex + rect->left]), color, rect->right - rect->left + 1);
        }
    }
}
void drawRect(Rect *rect, uint32_t bordercolor, uint32_t borderwidth)
{
    vbe_info_structure *pvbeinfo = g_vbebuff;
    vbe_mode_info_structure *pmodinfo = g_vbebuff + 512;
    uint32_t *pvbeframbuff = pmodinfo->PhysBasePtr;
    Rect line;
    // top line
    line.top = rect->top;
    line.left = rect->left;
    line.right = rect->right;
    line.bottom = rect->top + (borderwidth - 1);
    fillRect(&line, bordercolor);

    // right line
    line.top = rect->top;
    line.left = rect->right - (borderwidth - 1);
    line.right = rect->right;
    line.bottom = rect->bottom;
    fillRect(&line, bordercolor);

    // bottom line
    line.top = rect->bottom - (borderwidth - 1);
    line.left = rect->left;
    line.right = rect->right;
    line.bottom = rect->bottom;
    fillRect(&line, bordercolor);

    // left line
    line.top = rect->top;
    line.left = rect->left;
    line.right = rect->left + (borderwidth - 1);
    line.bottom = rect->bottom;
    fillRect(&line, bordercolor);
}
void drawBitmap(Rect *rect, Bitmap *bitmap)
{
    vbe_info_structure *pvbeinfo = g_vbebuff;
    vbe_mode_info_structure *pmodinfo = g_vbebuff + 512;
    if(!rectIsVaild(rect))
        return;
    Rect vaildrect;
    vaildrect.top = rect->top;
    vaildrect.left = rect->left;
    vaildrect.right = (rect->right >= pmodinfo->XResolution ? (pmodinfo->XResolution - 1) : rect->right);
    vaildrect.bottom = (rect->bottom >= pmodinfo->YResolution ? (pmodinfo->YResolution - 1) : rect->bottom);   
    if (bitmap)
    {
        if (bitmap->format == BITMAP_FORMAT_TYPE_R5G6B5)
        {
            if (pmodinfo->BitsPerPixel == 0x10) // 5:6:5
            {
                uint16_t *pvbeframbuff = pmodinfo->PhysBasePtr;
                uint16_t  *databuffptr = (uint16_t*)(bitmap->data);
                uint32_t yindex = vaildrect.top * pmodinfo->XResolution;
                uint32_t endheight = bitmap->height-1,linesize = bitmap->width; 
                if((vaildrect.top+endheight) > vaildrect.bottom)
                    endheight = vaildrect.bottom-vaildrect.top;
                if((vaildrect.left+linesize) > (vaildrect.right+1))
                    linesize = vaildrect.right - vaildrect.left+1;
                for (uint32_t i = 0; i <= endheight; ++i,databuffptr += bitmap->width,yindex += pmodinfo->XResolution)
                {
                    // for (uint32_t j = rect->left; j <= rect->right; ++j)
                    // {
                    //     pvbeframbuff[yindex + j] = color;
                    // }
                    memWordcpy_s(&(pvbeframbuff[yindex + vaildrect.left]), databuffptr, linesize);
                }
            }
            else if (pmodinfo->BitsPerPixel == 0x20) // 8:8:8:8
            {
            }
        }
        else if (bitmap->format == BITMAP_FORMAT_TYPE_A8R8G8B8)
        {
            if (pmodinfo->BitsPerPixel == 0x10) // 5:6:5
            {
            }
            else if (pmodinfo->BitsPerPixel == 0x20) // 8:8:8:8
            {
                uint32_t *pvbeframbuff = pmodinfo->PhysBasePtr;
                uint32_t  *databuffptr = (uint32_t*)(bitmap->data); 
                uint32_t yindex = vaildrect.top * pmodinfo->XResolution;
                uint32_t endheight = bitmap->height-1,linesize = bitmap->width; 
                if((vaildrect.top+endheight) > vaildrect.bottom)
                    endheight = vaildrect.bottom-vaildrect.top;
                if((vaildrect.left+linesize) > (vaildrect.right+1))
                    linesize = vaildrect.right - vaildrect.left+1;
                for (uint32_t i = 0; i <= endheight; ++i,databuffptr += bitmap->width,yindex += pmodinfo->XResolution)
                {
                    // for (uint32_t j = rect->left; j <= rect->right; ++j)
                    // {
                    //     pvbeframbuff[yindex + j] = color;
                    // }
                    memDWordcpy_s(&(pvbeframbuff[yindex + vaildrect.left]), databuffptr, linesize);
                }
            }
        }
    }
}
static void drawGTextBuff(uint32_t top, uint32_t left, uint32_t fillcolor)
{
    vbe_info_structure *pvbeinfo = g_vbebuff;
    vbe_mode_info_structure *pmodinfo = g_vbebuff + 512;
    if (pmodinfo->BitsPerPixel == 0x10) // 5:6:5
    {
        uint16_t color = 0;
        // r
        uint16_t temp = (uint16_t)((fillcolor >> 16) & 0xff);
        temp = (((temp & 0x1f) << 11));
        color |= temp;
        // g
        temp = (uint16_t)((fillcolor >> 8) & 0xff);
        temp = (((temp & 0x3f) << 5));
        color |= temp;
        // b
        temp = (uint16_t)((fillcolor)&0xff);
        temp = (temp & 0x1f);
        color |= temp;

        uint16_t *pvbeframbuff = pmodinfo->PhysBasePtr;
        uint32_t textbuffindex = 0, yindex = (top)*pmodinfo->XResolution;
        for (uint32_t i = 0; i < G_DRAW_TEXTBUFF_H; ++i, textbuffindex += G_DRAW_TEXTBUFF_W, yindex += pmodinfo->XResolution)
        {
            for (uint32_t j = 0; j < G_DRAW_TEXTBUFF_W; ++j)
            {
                if (g_DrawTextBuff[textbuffindex + j])
                {
                    pvbeframbuff[yindex + left + j] = color;
                }
            }
        }
    }
    else if (pmodinfo->BitsPerPixel == 0x20) // 8:8:8:8
    {
        uint32_t *pvbeframbuff = pmodinfo->PhysBasePtr;
        uint32_t textbuffindex = 0, yindex = (top)*pmodinfo->XResolution;
        uint32_t color = fillcolor & 0xffffff;
        for (uint32_t i = 0; i < G_DRAW_TEXTBUFF_H; ++i, textbuffindex += G_DRAW_TEXTBUFF_W, yindex += pmodinfo->XResolution)
        {
            for (uint32_t j = 0; j < G_DRAW_TEXTBUFF_W; ++j)
            {
                if (g_DrawTextBuff[textbuffindex + j])
                {
                    pvbeframbuff[yindex + left + j] = color;
                }
            }
        }
    }
}
void drawText(const char *text, Rect *rect, uint32_t color, float pixels)
{
    uint32_t len = strlen(text);
    char *bitmap = g_DrawTextBuff;
    /* 创建位图 */
    // int bitmap_w = rect->right - rect->left +1; /* 位图的宽 */
    // int bitmap_h = rect->bottom - rect->top+1; /* 位图的高 */
    int bitmap_w = G_DRAW_TEXTBUFF_W; /* 位图的宽 */
    int bitmap_h = G_DRAW_TEXTBUFF_H; /* 位图的高 */
    /* 计算字体缩放 */
    // float pixels = 12.0;                                    /* 字体大小（字号） */
    float scale = stbtt_ScaleForPixelHeight(g_fontinfo, pixels); /* scale = pixels / (ascent - descent) */

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
    stbtt_GetFontVMetrics(g_fontinfo, &ascent, &descent, &lineGap);
    /* 根据缩放调整字高 */
    ascent = roundf(ascent * scale);
    descent = roundf(descent * scale);

    int x = 0, y = 0; /*位图的x*/
    int advanceWidth = 0;
    int leftSideBearing = 0;
    int c_x1, c_y1, c_x2, c_y2;
    /* 调整字距 */
    int kern = 0;
    memDWordset_s(g_DrawTextBuff, 0, G_DRAW_TEXTBUFF_W * G_DRAW_TEXTBUFF_H / 4);
    for (int i = 0; i < len; i++)
    {
        /**
         * 获取水平方向上的度量
         * advanceWidth：字宽；
         * leftSideBearing：左侧位置；
         */
        advanceWidth = leftSideBearing = c_x1 = c_y1 = c_x2 = c_y2 = 0;

        stbtt_GetCodepointHMetrics(g_fontinfo, text[i], &advanceWidth, &leftSideBearing);

        /* 获取字符的边框（边界） */
        stbtt_GetCodepointBitmapBox(g_fontinfo, text[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

        /* 计算位图的y (不同字符的高度不同） */
        y = ascent + c_y1;

        /* 渲染字符 */
        int byteOffset = x + roundf(leftSideBearing * scale) + (y * bitmap_w);
        stbtt_MakeCodepointBitmap(g_fontinfo, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, bitmap_w, scale, scale, text[i]);

        /* 调整x */
        x += roundf(advanceWidth * scale);

        /* 调整字距 */
        int kern;
        kern = stbtt_GetCodepointKernAdvance(g_fontinfo, text[i], text[i + 1]);
        x += roundf(kern * scale);
    }
    drawGTextBuff(rect->top, rect->left, color);
   
}
/**
 * Parse PNG format into pixels. Returns NULL or error, otherwise the returned data looks like
 *   ret[2..] = 32 bit ARGB pixels (blue channel in the least significant byte, alpha channel in the most)
 */
// int png_parse(unsigned char *ptr, int size)
// {
//     int i, w, h, f;
//     unsigned char *img, *p;
//     stbi__context s;
//     stbi__result_info ri;
//     s.read_from_callbacks = 0;
//     s.img_buffer = s.img_buffer_original = ptr;
//     s.img_buffer_end = s.img_buffer_original_end = ptr + size;
//     ri.bits_per_channel = 8;
//     img = p = (unsigned char*)stbi__png_load(&s, (int*)&w, (int*)&h, (int*)&f, 1, &ri);
//     uint32_t *pdata = (uint32_t *)(g_BitmapCache.data);
//     // convert the returned image into frame buffer format
//     for(i = 0; i < w * h; i++, p += f)
//         switch(f) {
//             case 1: pdata[i] = 0xFF000000 | (p[0] << 16) | (p[0] << 8) | p[0]; break;
//             case 2: pdata[i] = (p[1] << 24) | (p[0] << 16) | (p[0] << 8) | p[0]; break;
//             case 3: pdata[i] = 0xFF000000 | (p[0] << 16) | (p[1] << 8) | p[2]; break;
//             case 4: pdata[i] = (p[3] << 24) | (p[0] << 16) | (p[1] << 8) | p[2]; break;
//         }
//     kernel_free(img);
//     g_BitmapCache.width = w;
//     g_BitmapCache.height = h;
//     g_BitmapCache.format= BITMAP_FORMAT_TYPE_A8R8G8B8;
//     return TRUE;
// }
void drawPngImage(Rect* rect,const char *filepath)
{
    FRESULT res; // 局部变量
    uint32_t br = 0, filesize = 0;
    FIL fp;
    res = f_open(&fp,filepath, FA_OPEN_ALWAYS | FA_READ);
    printf("open png file res = %d \n", res);
    if (res == FR_OK)
    {
        filesize = f_size(&fp);
        printf("png file size 0x%x \n", filesize);
        res = f_read(&fp, g_BitmapCache.data, filesize, &br);
        f_close(&fp);
        if (res == FR_OK)
        {
            printf(" read png data size %x\n", br);
        }else{
            return;
        }  
    } 
    g_BitmapCache.format = BITMAP_FORMAT_TYPE_A8R8G8B8;
    g_BitmapCache.width = rect->right-rect->left+1;
    g_BitmapCache.height = rect->bottom-rect->top+1;
    drawBitmap(rect, &g_BitmapCache);
}