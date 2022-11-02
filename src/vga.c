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
static const u8 width_400[] = {0x70, 0x63, 0x64, 0x92, 0x65, 0x82, 0x32};

static const u8 ver_regs[] = {0x6, 0x7, 0x9, 0x10, 0x11, 0x12, 0x15, 0x16};

static const u8 height_200[] = {0xbf, 0x1f, 0x41, 0x9c, 0x8e, 0x8f, 0x96, 0xb9};
static const u8 height_224[] = {0x0b, 0x3e, 0x41, 0xda, 0x9c, 0xbf, 0xc7, 0x04};
static const u8 height_240[] = {0x0d, 0x3e, 0x41, 0xea, 0xac, 0xdf, 0xe7, 0x06};
static const u8 height_256[] = {0x23, 0xb2, 0x61, 0x0a, 0xac, 0xff, 0x07, 0x1a};
static const u8 height_270[] = {0x30, 0xf0, 0x61, 0x20, 0xa9, 0x1b, 0x1f, 0x2f};
static const u8 height_300[] = {0x70, 0xf0, 0x61, 0x5b, 0x8c, 0x57, 0x58, 0x70};
static const u8 height_360[] = {0xbf, 0x1f, 0x40, 0x88, 0x85, 0x67, 0x6d, 0xba};
static const u8 height_400[] = {0xbf, 0x1f, 0x40, 0x9c, 0x8e, 0x8f, 0x96, 0xb9};
static const u8 height_480[] = {0x0d, 0x3e, 0x40, 0xea, 0xac, 0xdf, 0xe7, 0x06};
static const u8 height_564[] = {0x62, 0xf0, 0x60, 0x37, 0x89, 0x33, 0x3c, 0x5c};
static const u8 height_600[] = {0x70, 0xf0, 0x60, 0x5b, 0x8c, 0x57, 0x58, 0x70};

// the chain4 parameter should be 1 for normal 13h-type mode, but
// only allows 320x200 256x200, 256x240 and 256x256 because you
// can only access the first 64kb

// if chain4 is 0, then plane mode is used (tweaked modes), and
// you'll need to switch planes to access the whole screen but
// that allows you using any resolution, up to 400x600

#define outp outb
#define outpw outw
#define inp inb
#define inpw inw

#define RGB_RESET 0x03C6
#define RGB_READ 0x03C7
#define RGB_WRITE 0x03C8
#define RGB_DATA 0x03C9

// returns 1=ok, 0=fail
u32 init_graph_vga(u32 width, u32 height, b32 chain4)
{
    const u8* w;
    const u8* h;
    u8 val;
    u32 a;

    switch (width) {
        case 256:
            w = width_256;
            val = R_COM + R_W256;
            break;
        case 320:
            w = width_320;
            val = R_COM + R_W320;
            break;
        case 360:
            w = width_360;
            val = R_COM + R_W360;
            break;
        case 376:
            w = width_376;
            val = R_COM + R_W376;
            break;
        case 400:
            w = width_400;
            val = R_COM + R_W400;
            break;
        default:
            return 0;  // fail
    }
    switch (height) {
        case 200:
            h = height_200;
            val |= R_H200;
            break;
        case 224:
            h = height_224;
            val |= R_H224;
            break;
        case 240:
            h = height_240;
            val |= R_H240;
            break;
        case 256:
            h = height_256;
            val |= R_H256;
            break;
        case 270:
            h = height_270;
            val |= R_H270;
            break;
        case 300:
            h = height_300;
            val |= R_H300;
            break;
        case 360:
            h = height_360;
            val |= R_H360;
            break;
        case 400:
            h = height_400;
            val |= R_H400;
            break;
        case 480:
            h = height_480;
            val |= R_H480;
            break;
        case 564:
            h = height_564;
            val |= R_H564;
            break;
        case 600:
            h = height_600;
            val |= R_H600;
            break;
        default:
            return 0;  // fail
    }

    // chain4 not available if mode takes over 64k

    if (chain4 && width * height > 65536L) return 0;

    // here goes the actual modeswitch

    outp(0x3c2, val);
    outpw(0x3d4, 0x0e11);  // enable regs 0-7

    for (a = 0; a < SZ(hor_regs); ++a)
        outpw(0x3d4, (word)((w[a] << 8) + hor_regs[a]));
    for (a = 0; a < SZ(ver_regs); ++a)
        outpw(0x3d4, (word)((h[a] << 8) + ver_regs[a]));

    outpw(0x3d4, 0x0008);  // vert.panning = 0

    if (chain4) {
        outpw(0x3d4, 0x4014);
        outpw(0x3d4, 0xa317);
        outpw(0x3c4, 0x0e04);
    } else {
        outpw(0x3d4, 0x0014);
        outpw(0x3d4, 0xe317);
        outpw(0x3c4, 0x0604);
    }

    outpw(0x3c4, 0x0101);
    outpw(0x3c4, 0x0f02);  // enable writing to all planes
    outpw(0x3ce, 0x4005);  // 256color mode
    outpw(0x3ce, 0x0506);  // graph mode & A000-AFFF

    inp(0x3da);

    outp(0x3c0, 0x30);
    outp(0x3c0, 0x41);
    outp(0x3c0, 0x33);
    outp(0x3c0, 0x00);

    for (a = 0; a < 16; a++) {  // ega pal
        outp(0x3c0, (byte)a);
        outp(0x3c0, (byte)a);
    }

    // setup extended palette

    outp(0x3c0, 0x20);  // enable video

    vga_screen_width = width;
    vga_screen_height = height;
    vga_screen_colors = 255;
    vga_is_chained = chain4 == 1 ? true : false;

    return 1;
}

void set_palette(int index, int red, int green, int blue)
{
    outp(RGB_RESET, 0xFF);  // Prepare the VGA card for the color change
    outp(RGB_WRITE, index);  // Tell which palette register to write to
    // The following values can be anywhere from 0 to 63
    outp(RGB_DATA, red);  // change the red value
    outp(RGB_DATA, green);  // change the green value
    outp(RGB_DATA, blue);  // change the blue value
}

// void vga_wait_for_retrace(void)
// {
//   /* wait until done with vertical retrace */
//   while  ((inp(INPUT_STATUS) & VRETRACE));
//   /* wait until done refreshing */
//   while (!(inp(INPUT_STATUS) & VRETRACE));
// }

void vga_clear(u8 color)
{
    // TODO: vga_clear(1);
    kmemsetb((u8*)vga_memory, color, vga_screen_width * vga_screen_height);

    // reset
    outp(0x03C6, 0xff);
    outp(0x03C8, 0x10);
    for (u8 i = 0x10; i < 0xff; i++) {
        // outp(0x03c8, i);
        // r = 1-16
        // g = 1x4-16x4
        // b = 1x16-16x16
        outp(0x03c9, i % 16 * 16);
        outp(0x03c9, i / 4 * 16);
        outp(0x03c9, i / 16 * 16);
    }
}

void plot_pixel(u32 x, u32 y, u8 color)
{
    // TODO: check we are in video mode?

    /*  y*320 = y*256 + y*64 = y*2^8 + y*2^6   */
    vga_memory[(y << 8) + (y << 6) + x] = color;

    // TODO : vga_memory[ y * screen_width + x] = color;
}

i32 abs(i32 val) { return val < 0 ? -val : val; }
i32 sgn(i32 val) { return val < 0 ? -1 : val > 0 ? 1 : 0; }

void line_fast(u32 x1, u32 y1, u32 x2, u32 y2, u8 color)
{
    i32 i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

    dx = x2 - x1; /* the horizontal distance of the line */
    dy = y2 - y1; /* the vertical distance of the line */
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = sgn(dx);
    sdy = sgn(dy);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;

    vga_memory[(py << 8) + (py << 6) + px] = color;

    if (dxabs >= dyabs) /* the line is more horizontal than vertical */
    {
        for (i = 0; i < dxabs; i++) {
            y += dyabs;
            if (y >= dxabs) {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;
            plot_pixel(px, py, color);
        }
    } else /* the line is more vertical than horizontal */
    {
        for (i = 0; i < dyabs; i++) {
            x += dxabs;
            if (x >= dyabs) {
                x -= dyabs;
                px += sdx;
            }
            py += sdy;
            plot_pixel(px, py, color);
        }
    }
}

void polygon(u32 num_vertices, u32* vertices, u8 color)
{
    for (u32 i = 0; i < num_vertices - 1; i++)
    {
        line_fast(vertices[(i << 1) + 0], vertices[(i << 1) + 1],
                  vertices[(i << 1) + 2], vertices[(i << 1) + 3], color);
    }
    // end to start
    line_fast(vertices[0], vertices[1], vertices[(num_vertices << 1) - 2],
              vertices[(num_vertices << 1) - 1], color);
}

void fillrect(u32 xoff, u32 yoff, u8 color)
{
    u32 height = 10;
    u32 width = 10;
    for (u32 y = 0; y < height; ++y) {
        for (u32 x = 0; x < width; ++x) {
            plot_pixel(xoff + x, yoff + y, color);
        }
    }
}

void vga_tests()
{
    // clear
    vga_clear(3);

    delay_s(1);

    set_palette(58, 255, 20, 20);
    vga_clear(58);

    delay_s(1);
    
    // plot pixels
    u8 color = 0;
    for (u32 y = 0; y < vga_screen_height; ++y) {
        for (u32 x = 0; x < vga_screen_width; ++x) {
            plot_pixel(x, y, color);
        }
        color = (color + 1) % 255;  // vga_screen_colors;
    }
    
    delay_s(1);
    
    //  // randomly draw 5000 lines.
    //  u32 x1, y1, x2, y2;
    //  for (u32 i = 0; i < 1000; i++) {
    //    x1 = rand() % vga_screen_width;
    //    y1 = rand() % vga_screen_height;
    //    x2 = rand() % vga_screen_width;
    //    y2 = rand() % vga_screen_height;
    //    color = rand() % vga_screen_colors;
    //    line_fast(x1, y1, x2, y2, color);
    //  }
    //
    //  delay_s(1);
    
    // polygon
    u32 num_vertices = 3;
    u32 vertices[6] = {5, 0,  /* (x1,y1) */
        7, 5,  /* (x2,y2) */
        1, 4}; /* (x3,y3) */
    polygon(num_vertices, vertices, 15);
}





















////////////////////////////////////////////////////////////



#define u8int u8
#define u16int u16
#define u32int u32


#define VGA_AC_INDEX        0x3C0
#define VGA_AC_WRITE        0x3C0
#define VGA_MISC_WRITE      0x3C2
#define VGA_SEQ_INDEX       0x3C4
#define VGA_SEQ_DATA        0x3C5
#define VGA_GC_INDEX        0x3CE
#define VGA_GC_DATA         0x3CF
#define VGA_CRTC_INDEX      0x3D4
#define VGA_CRTC_DATA       0x3D5
#define VGA_INSTAT_READ     0x3DA
#define VGA_NUM_SEQ_REGS    5
#define VGA_NUM_CRTC_REGS   25
#define VGA_NUM_GC_REGS     9
#define VGA_NUM_AC_REGS     21

void swich_to_8x8_mode();


u8int g_80x50_text[] =
{
/* MISC */
    0x67,
/* SEQ */
    0x03, 0x00, 0x03, 0x00, 0x02,
/* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
    0x00, 0x47, 0x06, 0x07, 0x00, 0x00, 0x01, 0x40,
    0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF, 
/* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0xFF, 
/* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00,
};

u8int g_8x8_font[2048] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7E, 0x81, 0xA5, 0x81, 0xBD, 0x99, 0x81, 0x7E,
    0x7E, 0xFF, 0xDB, 0xFF, 0xC3, 0xE7, 0xFF, 0x7E,
    0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00,
    0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00,
    0x38, 0x7C, 0x38, 0xFE, 0xFE, 0x92, 0x10, 0x7C,
    0x00, 0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x7C,
    0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00, 0x00,
    0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF,
    0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00,
    0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF,
    0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78,
    0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18,
    0x3F, 0x33, 0x3F, 0x30, 0x30, 0x70, 0xF0, 0xE0,
    0x7F, 0x63, 0x7F, 0x63, 0x63, 0x67, 0xE6, 0xC0,
    0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99,
    0x80, 0xE0, 0xF8, 0xFE, 0xF8, 0xE0, 0x80, 0x00,
    0x02, 0x0E, 0x3E, 0xFE, 0x3E, 0x0E, 0x02, 0x00,
    0x18, 0x3C, 0x7E, 0x18, 0x18, 0x7E, 0x3C, 0x18, 
    0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00, 
    0x7F, 0xDB, 0xDB, 0x7B, 0x1B, 0x1B, 0x1B, 0x00, 
    0x3E, 0x63, 0x38, 0x6C, 0x6C, 0x38, 0x86, 0xFC, 
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x7E, 0x00, 
    0x18, 0x3C, 0x7E, 0x18, 0x7E, 0x3C, 0x18, 0xFF,
    0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00,
    0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00,
    0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00, 
    0x00, 0x30, 0x60, 0xFE, 0x60, 0x30, 0x00, 0x00, 
    0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xFE, 0x00, 0x00, 
    0x00, 0x24, 0x66, 0xFF, 0x66, 0x24, 0x00, 0x00, 
    0x00, 0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x00, 0x00, 
    0x00, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00,
    0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00, 
    0x18, 0x7E, 0xC0, 0x7C, 0x06, 0xFC, 0x18, 0x00, 
    0x00, 0xC6, 0xCC, 0x18, 0x30, 