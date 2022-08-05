//
// vga mode switcher by Jonas Berlin -98 <jberlin@cc.hut.fi>
//

#include <system.h>

typedef char byte;
typedef unsigned short word;
typedef u32 dword;

//////////////////////////////////////////////////////////////////

u8* vga_memory = (u8*)0xA0000;
u16 vga_screen_width = 320;
u16 vga_screen_height = 200;
u8 vga_screen_colors = 255;

#define MAX_PALETTE_COUNT 256
b8 vga_is_chained = false;
u8 vga_palette[MAX_PALETTE_COUNT];

//////////////////////////////////////////////////////////////////

#define SZ(x) (sizeof(x) / sizeof(x[0]))

// misc out (3c2h) value for various modes

#define R_COM 0x63  // "common" bits

#define R_W256 0x00
#define R_W320 0x00
#define R_W360 0x04
#define R_W376 0x04
#define R_W400 0x04

#define R_H200 0x00
#define R_H224 0x80
#define R_H240 0x80
#define R_H256 0x80
#define R_H270 0x80
#define R_H300 0x80
#define R_H360 0x00
#define R_H400 0x00
#define R_H480 0x80
#define R_H564 0x80
#define R_H600 0x80

static const u8 hor_regs[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x13};

static const u8 width_256[] = {0x5f, 0x3f, 0x40, 0x82, 0x4a, 0x9a, 0x20};
static const u8 width_320[] = {0x5f, 0x4f, 0x50, 0x82, 0x54, 0x80, 0x28};
static const u8 width_360[] = {0x6b, 0x59, 0x5a, 0x8e, 0x5e, 0x8a, 0x2d};
static const u8 width_376[] = {0x6e, 0x5d, 0x5e, 0x91, 0x62, 0x8f, 0x2f};
static