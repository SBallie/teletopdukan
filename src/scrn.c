
#include <system.h>


////////////////////////////////////////////////////////////////////////////////

/*
 * These define our textpointer, our background and foreground
 * colors (attributes), and x and y cursor coordinates
 */
u16* textmemptr;		// word pointer
u16  attrib = 0x0F;		// attribute for text colors
u16  csr_x = 0;
u16  csr_y = 0;	// since these shouldn't ever be neg.

char palette16[16];
char palette256[768];

u16* vidmemptr;        // word pointer

b32 write_to_serial = false;
b32 write_to_stdout = false;

// 'cursor' since last kwritef
internal int cursor = 0;


////////////////////////////////////////////////////////////////
// TEXT MODE

/* Sets the forecolor and backcolor that we will use */
void set_text_color(u8 forecolor, u8 backcolor)
{
    /* Top 4 bytes are the background, bottom 4 bytes
     *  are the foreground color */
    attrib = (backcolor << 4) | (forecolor & 0x0F);
}

/* Sets our text-mode VGA pointer, then clears the screen for us */
void init_video()
{
    textmemptr = (u16*)0xB8000;
    vidmemptr = (u16*)0xA0000;
    cls();
}

/* Scrolls the screen */
void kscroll(void)
{
    u16 blank;
    u32 temp;

    /* A blank is defined as a space... we need to give it
     *  backcolor too */
    blank = 0x20 | (attrib << 8);

    /* Row 25 is the end, this means we need to scroll up */
    if(csr_y >= 25)
    {
        /* Move the current text chunk that makes up the screen
         *  back in the buffer by a line */
        temp = csr_y - 25 + 1;
        kmemcpyb((u8*)textmemptr, (u8*)(textmemptr + temp * 80), (25 - temp) * 80 * 2);

        /* Finally, we set the chunk of memory that occupies
         *  the last line of text to our 'blank' character */
        kmemsetw((u16*)(textmemptr + (25 - temp) * 80), blank, 80);
        csr_y = 25 - 1;
    }
}

/* Updates the hardware cursor: the little blinking line
 *  on the screen under the last character pressed! */
void move_csr(void)
{
    u32 temp;

    /* The equation for finding the index in a linear
     *  chunk of memory can be represented by:
     *  Index = [(y * width) + x] */
    temp = csr_y * 80 + csr_x;

    /* This sends a command to indicies 14 and 15 in the
     *  CRT Control Register of the VGA controller. These
     *  are the high and low bytes of the index that show
     *  where the hardware cursor is to be 'blinking'. To
     *  learn more, you should look up some VGA specific
     *  programming documents. A great start to graphics:
     *  http://www.brackeen.com/home/vga */
    outb(0x3D4, 14);
    outb(0x3D5, temp >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, temp);
}

/* Clears the screen */
void cls()
{
    u16 blank;
    int i;

    /* Again, we need the 16-bits that will be used to
     *  represent a space with color */
    blank = 0x20 | (attrib << 8);

    /* Sets the entire screen to spaces in our current
     *  color */
    for(i = 0; i < 25; i++)
        kmemsetw(textmemptr + i * 80, blank, 80);

    /* Update our virtual cursor, and then move the
     *  hardware cursor */
    csr_x = 0;