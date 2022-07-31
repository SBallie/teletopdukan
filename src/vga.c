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

// misc out (3c2