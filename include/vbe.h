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
    uint16_t attributes;  // deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
    uint8_t window_a;     // deprecated
    uint8_t window_b;     // deprecated
    uint16_t granularity; // deprecated; used while calculating bank numbers
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr; // deprecated; used to switch banks from protected mode without returning to real mode
    uint16_t pitch;        // number of bytes per horizontal line
    uint16_t width;        // width in pixels
    uint16_t height;       // height in pixels
    uint8_t w_char;        // unused...
    uint8_t y_char;        // ...
    uint8_t planes;
    uint8_t bpp;   // bits per pixel in this mode
    uint8_t banks; // deprecated; total number of banks in this mode
    uint8_t memory_model;
    uint8_t bank_size; // deprecated; size of a bank, almost always 64 KB but may be 16 KB...
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer; // physical address of the linear frame buffer; write here to draw to the screen
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size; // size of memory in the framebuffer but not being displayed on the screen
    uint8_t reserved1[206];
} vbe_mode_info_structure;

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

#define VBE_BUFF_SIZE 768
void dumpVbeInfo();
extern char g_vbebuff[VBE_BUFF_SIZE];
#endif