#ifndef _VBE_H_H
#define _VBE_H_H
#include "stdint.h"
#pragma pack(1)
typedef struct vbe_info_structure
{
    char signature[4];     // = "VESA";	// must be "VESA" to indicate valid VBE support
    uint16_t version;      // VBE version; high byte is major version, low byte is minor version
    uint32_t oem;          // segment:offset pointer to OEM
    uint32_t capabilities; // bitfield that describes card capabilities
    uint32_t video_modes;  // segment:offset pointer to list of supported video modes
    uint16_t video_memory; // amount of video memory in 64KB blocks
    uint16_t software_rev; // software revision
    uint32_t vendor;       // segment:offset to card vendor string
    uint32_t product_name; // segment:offset to card model name
    uint32_t product_rev;  // segment:offset pointer to product revision
    char reserved[222];    // reserved for future expansion
    char oem_data[256];    // OEM BIOSes store their strings in this area
} vbe_info_structure;

typedef struct vbe_mode_info_structure
{
//; Mandatory information for all VBE revisions
uint16_t ModeAttributes;//; mode attributes
uint8_t WinAAttributes ;//; window A attributes
uint8_t WinBAttributes ;//; window B attributes
uint16_t WinGranularity ;//; window granularity
uint16_t WinSize;//; window size
uint16_t WinASegment;//; window A start segment
uint16_t WinBSegment;//; window B start segment
uint32_t WinFuncPtr;//; real mode pointer to window function
uint16_t BytesPerScanLine;//; bytes per scan line
// Mandatory information for VBE 1.2 and above
uint16_t XResolution;//; horizontal resolution in pixels or characters3
uint16_t YResolution;//; vertical resolution in pixels or characters
uint8_t XCharSize ;//; character cell width in pixels
uint8_t YCharSize ;//; character cell height in pixels
uint8_t NumberOfPlanes ;//; number of memory planes
uint8_t BitsPerPixel ;//; bits per pixel
uint8_t NumberOfBanks ;//; number of banks
uint8_t MemoryModel ;//; memory model type
uint8_t BankSize ;//; bank size in KB
uint8_t NumberOfImagePages ;//; number of images
uint8_t Reserved0; //reserved for page function
// Direct Color fields (required for direct/6 and YUV/7 memory models)
uint8_t RedMaskSize;//; size of direct color red mask in bits
uint8_t RedFieldPosition ;//; bit position of lsb of red mask
uint8_t GreenMaskSize ;//; size of direct color green mask in bits
uint8_t GreenFieldPosition ;//; bit position of lsb of green mask
uint8_t BlueMaskSize ;//; size of direct color blue mask in bits
uint8_t BlueFieldPosition ;//; bit position of lsb of blue mask
uint8_t RsvdMaskSize ;//; size of direct color reserved mask in bits
uint8_t RsvdFieldPosition ;//; bit position of lsb of reserved mask
uint8_t DirectColorModeInfo ;//; direct color mode attributes
// Mandatory information for VBE 2.0 and above
uint32_t PhysBasePtr;//; physical address for flat memory frame buffer
uint32_t Reserved1; //Reserved - always set to 0
uint16_t Reserved2; //Reserved - always set to 0

// Mandatory information for VBE 3.0 and above
uint16_t LinBytesPerScanLine ;//; bytes per scan line for linear modes
uint8_t BnkNumberOfImagePages ;//; number of images for banked modes
uint8_t LinNumberOfImagePages ;//; number of images for linear modes
uint8_t LinRedMaskSize ;//; size of direct color red mask (linear modes)
uint8_t LinRedFieldPosition ;//; bit position of lsb of red mask (linear modes)
uint8_t LinGreenMaskSize ;//; size of direct color green mask (linear modes)
uint8_t LinGreenFieldPosition;//; bit position of lsb of green mask (linear modes)
uint8_t LinBlueMaskSize ;//; size of direct color blue mask (linear modes)
uint8_t LinBlueFieldPosition ;//; bit position of lsb of blue mask (linear modes)
uint8_t LinRsvdMaskSize ;//; size of direct color reserved mask (linear modes)
uint8_t LinRsvdFieldPosition ;//; bit position of lsb of reserved mask (linear modes)
uint32_t MaxPixelClock ;//; maximum pixel clock (in Hz) for graphics mode
//Reserved uint8_t 189 dup (?) ; remainder of ModeInfoBlock
}vbe_mode_info_structure;

typedef struct PMInfoBlock
{
    uint8_t  Signature[4];     // db 'PMID' ; PM Info Block Signature
    uint16_t EntryPoint;   // dw ? ; Offset of PM entry point within BIOS
    uint16_t PMInitialize; // dw ? ; Offset of PM initialization entry point
    uint16_t BIOSDataSel;  // dw 0 ; Selector to BIOS data area emulation block
    uint16_t A0000Sel;     // dw A000h ; Selector to access A0000h physical mem
    uint16_t B0000Sel;     // dw B000h ; Selector to access B0000h physical mem
    uint16_t B8000Sel;     // dw B800h ; Selector to access B8000h physical mem
    uint16_t CodeSegSel;   // dw C000h ; Selector to access code segment as data
    uint8_t InProtectMode; // db 0 ; Set to 1 when in protected mode
    uint8_t Checksum;      // db ? ; Checksum byte for structure
}PMInfoBlock;

#pragma pack()

#define VBE_BUFF_SIZE 2048
void initVbe();
extern char g_vbebuff[VBE_BUFF_SIZE];

void initFont();

typedef struct Rect
{
    uint32_t top;
    uint32_t left;
    uint32_t bottom;
    uint32_t right;
}Rect;
typedef struct Point
{
    uint32_t x;
    uint32_t y;
}Point,Pair;


typedef enum BITMAP_FORMAT_TYPE
{
    BITMAP_FORMAT_TYPE_R5G6B5 =0,
    BITMAP_FORMAT_TYPE_A8R8G8B8
}BITMAP_FORMAT_TYPE;
typedef struct Bitmap
{
    uint8_t *data;
    BITMAP_FORMAT_TYPE format;
    uint32_t width;
    uint32_t height;
}Bitmap;
void getScreenPixSize(Pair* size);
void fillRect(Rect* rect, uint32_t fillcolor);
void drawRect(Rect* rect, uint32_t bordercolor, uint32_t borderwidth);
void drawBitmap(Rect* rect,Bitmap *data);
void drawText(const char* text, Rect* rect, uint32_t color, float pixels);
int  rectIsVaild(Rect* rect);
void drawPngImage(Rect* rect,const char *filepath);
#endif