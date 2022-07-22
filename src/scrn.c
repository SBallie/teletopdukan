
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
    csr_y = 0;
    move_csr();
}

// TODO: use STDIN or terminal input buffer, read next byte if exists, else wait
u8 kgetch()
{
    u8 ch = keyboard_read_next();
    while(!ch)
    {
        // TODO: sleep to reduce spinwait
        ch = keyboard_read_next();
    }
    return ch;
}

// TODO: should store into a buffer instead
/* Puts a single character on the screen */
void kputch(u8 c)
{
    u16 *where;
    u16 att = attrib << 8;

    /* Handle a backspace, by moving the cursor back one space */
    if(c == 0x08)
    {
        if(csr_x != 0)
            --csr_x;
        if(cursor != 0)
            --cursor;
    }
    /* Handles a tab by incrementing the cursor's x, but only
     *  to a point that will make it divisible by 8 */
    else if(c == 0x09)
    {
        csr_x = (csr_x + 8) & ~(8 - 1);
        cursor = (cursor + 8) & ~(8 - 1);
    }
    /* Handles a 'Carriage Return', which simply brings the
     *  cursor back to the margin */
    else if(c == '\r')
    {
        csr_x = 0;
        cursor = 0;
    }
    /* We handle our newlines the way DOS and the BIOS do: we
     *  treat it as if a 'CR' was also there, so we bring the
     *  cursor to the margin and we increment the 'y' value */
    else if(c == '\n')
    {
        csr_x = 0;
        cursor += 80;
        csr_y++;
    }
    /* Any character greater than and including a space, is a
     *  printable character. The equation for finding the index
     *  in a linear chunk of memory can be represented by:
     *  Index = [(y * width) + x] */
    else if(c >= ' ')
    {
        where = textmemptr + (csr_y * 80 + csr_x);
        *where = c | att;	/* Character AND attributes: color */
        ++csr_x;
        ++cursor;
    }

    /* If the cursor has reached the edge of the screen's width, we
     *  insert a new line in there */
    if(csr_x >= 80)
    {
        csr_x = 0;
        cursor += 80;
        csr_y++;
    }

    /* Scroll the screen if needed, and finally move the cursor */
    kscroll();
    move_csr();
}

void printInt(i32 l) { writeInt(kputch, l); }
void printHex(u32 l) { writeHex(kputch, l); }
void printHex_w(u16 w) { writeHex_w(kputch, w); }
void printHex_b(u8 b) { writeHex_b(kputch, b); }
void printAddr(void* addr) { writeAddr(kputch, addr); }
void printBinary_b(u8 num) { writeBinary_b(kputch, num); }

// u32 - 10, u64 - 20
#define MAX_INT_DIGITS 24

// TODO: should allow wrap on word
// TODO: should allow padding (at least with (0) zeros)
// Note: assume base 10 for right now
void writeInt(output_writer writer, i32 num)
{
    u8 delim_negative = '-';
    if(num < 0) {
        writer(delim_negative);
        writeUInt(writer, (u32)-num);
    } else {
        writeUInt(writer, (u32)num);
    }
    return;
}

/*

 print u64

 .data
 decstr  db      24 dup (0)