
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
 pfstr   db      '%','s',0dh,0ah,0
 .code
 extrn   _printf:NEAR
 _main   proc    near
 mov     edi,000000002h          ;edi = high order dvnd
 mov     esi,04CB016EAh          ;esi = low  order dvnd
 lea     ebx,decstr+23           ;ebx = ptr to end string
 mov     ecx,10                  ;ecx = 10 (constant)
 div0:   xor     edx,edx                 ;clear edx
 mov     eax,edi                 ;divide high order
 div     ecx
 mov     edi,eax
 mov     eax,esi                 ;divide low order
 div     ecx
 mov     esi,eax
 add     dl,'0'                  ;store ascii digit
 dec     ebx
 mov     [ebx],dl
 mov     eax,edi                 ;repeat till dvnd == 0
 or      eax,esi
 jnz     div0
 push    ebx                     ;display string
 push    offset pfstr
 call    _printf
 add     sp,8
 xor     eax,eax
 ret
 _main   endp


 asm ("movl %0,%%eax;
 movl %1,%%ecx;
 call _foo"
 :
 : "g" (from), "g" (to)
 : "eax", "ecx"
 );


 */


// TODO: need to link in libgcc (should have been with compiler build)
//
// http://www.delorie.com/gnu/docs/gcc/gcc_80.html
// http://www.ic.unicamp.br/~islene/2s2008-mo806/libc/sysdeps/wordsize-32/divdi3.c
//
// HACK: subtrack out divisor until left with A
// - https://stackoverflow.com/questions/2566010/fastest-way-to-calculate-a-128-bit-integer-modulo-a-64-bit-integer?rq=1
//

typedef struct { u64 quotient; u64 remainder; } div_t;
u64 modulo(u64 A, u64 B)
{
    u64 X = B;

    // find a decent divisor
    while (X < A/2) {
        X <<= 1;
    }

    // divide by subtracting, remainders left in A
    while (A >= B) {
        if (A >= X)
            A -= X;
        X >>= 1;
    }
    return A;
}

// STEVE: simple SLOW version of division
div_t div64_slow(u64 dividend, u64 divisor)
{
    ASSERT(divisor, "can't divide with zero divisor!");
    // TODO: speed up a bit by testing the divisor size
    // - for large dividend and small divisor can look at first
    // - subtracting a large multiple of divisor

    u64 quotient = 0;
    u64 quotient_per_removal = 1;
    // find a decent divisor
    u64 A = dividend;
    u64 B = divisor;
    u64 X = B;
    while (X < A/2) {
        X <<= 1; // mult by two
        //        ++quotient_per_removal;
        quotient_per_removal <<= 1;
    }

    // divide by subtracting, remainders left in A
    while (A >= B) {
        if (A >= X) {
            A -= X;
            quotient += quotient_per_removal;
        }
        X >>= 1;
        quotient_per_removal >>= 1;
    }

    div_t ret = { quotient, A };
    return ret;
}

void writeUInt64(output_writer writer, u64 num)
{
    if(num == 0) {
        writer('0');
        return;
    }

    u8 buf[MAX_INT_DIGITS + 1];
    u8 cur = MAX_INT_DIGITS;
    buf[cur] = '\0';

    while(num) {
        --cur;
        div_t qr = div64_slow(num, 10);
        num = qr.quotient;
        buf[cur] = qr.remainder + '0'; // add the digit (temp) to ASCII code '0'
    }

    // print the string
    char c = '0';
    for(; cur < MAX_INT_DIGITS; ++cur) {
        c = buf[cur];
        writer(c);
    }
}

void writeInt64(output_writer writer, i64 num)
{
    u8 delim_negative = '-';
    if(num < 0) {
        writer(delim_negative);
        num = -num;
    }
    writeUInt64(writer, (u64)num);
}

void writeUInt(output_writer writer, u32 num)
{
    u8 buf[MAX_INT_DIGITS];
    i32 cur, end, temp=0;

    end = MAX_INT_DIGITS-1;
    cur = end;

    if(num == 0) {
        writer('0');
        return;
    }

    buf[cur] = '\0';

    while(num) {
        temp = num % 10; // get 'ones' or right most digit
        --cur;
        buf[cur] = temp + '0'; // add the digit (temp) to ASCII code '0'
        num /= 10; // remove right most digit
    }

    // print the string
    char c = '0';
    for(; cur < end; ++cur) {
        c = buf[cur];
        writer(c);
    }
}

static inline unsigned bit_mask(int x)
{
    return (x >= sizeof(unsigned) * CHAR_BIT) ? (unsigned) -1 : (1U << x) - 1;
}

void writeRealDebug(output_writer writer, real64 num)
{
    // NOTE: Intel Double Precision
    // NOTE: need to be careful to not get automatic value conversion instead of just type conversion
    u64* numToIntPtr = (u64*)&num;
    u64 numInt = *numToIntPtr;

    u8  signbit = (numInt >> 63) & 1;
    u16 exp = (numInt >> 52) & bit_mask(11);
    u16 expbias = 1023;
    u16 exponent = exp - expbias;
    u64 mantissa = numInt & bit_mask(52);

    if(exponent == 0) {
        kwrites(writer, "signed zero (mantissa = 0) otherwise subnormal\n");
    }
    if(exponent == 0x7ff) {
        kwrites(writer, "infinity (mantissa = 0) otherwise NaN\n");
    }

#if (DEBUG_LEVEL > 1)

    writer('\n');
    writer('[');
    writeHex_q(writer, numInt);
    writer(':');
    writer(' ');
    writeHex_b(writer, signbit);
    writer(',');
    writeHex_w(writer, exp);
    kwrites(writer," => ");
    writeInt(writer, exponent);
    writer(',');
    writeHex_q(writer, mantissa);
    kwrites(writer," => ");
    writeInt(writer, mantissa);
    writer(']');
    writer('\n');

#endif

    if(signbit) writer('-');
    kwrites(writer, "1.");
    writeUInt64(writer, mantissa);
    kwrites(writer, " * 2^");
    writeInt(writer, (i32)exponent);

    return;
}

#define scast(typ) (typ)
#define MAX_FLOATING_FRACTIONAL_DIGITS 10

void writeReal_component(output_writer writer, real64 smallNum, bool write_fraction)
{
    if(smallNum < 0) {
        smallNum = -smallNum;
        writer('-');
    }

    // HACK: cast to truncate
    i64 whole = (i64)smallNum;
    real64 fractional = smallNum - (real64)whole;