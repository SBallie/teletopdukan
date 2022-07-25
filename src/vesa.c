
#include <system.h>

// TODO: http://codereview.stackexchange.com/questions/108168/vbe-bdf-font-rendering

typedef struct PACKED
{
    u8  VbeSignature[4];             // == "VESA"
    u16 VbeVersion;                 // == 0x0300 for VBE 3.0
    u16 OemStringPtr[2];            // isa vbeFarPtr
    u16 Capabilities[4];
    u16 VideoModePtr[2];         // isa vbeFarPtr
    u16 TotalMemory;             // as # of 64KB blocks
} VbeInfoBlock;

typedef struct PACKED
{
    u16 attributes;
    u8 winA,winB;
    u16 granularity;
    u16 winsize;
    u16 segmentA, segmentB;

    //TODO: VBE_FAR(realFctPtr);

    u16 pitch; // bytes per scanline

    u16 Xres, Yres;
    u8 Wchar, Ychar, planes, bpp, banks;
    u8 memory_model, bank_size, image_pages;
    u8 reserved0;
