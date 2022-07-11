
// /**/
//
// Stream Setup and Prep:
// - Practice Streaming Something Simple
// - Decide on one or two major features "unique" to the OS
// - Create a list of topics for the first 10 streams
// - Decide on rough time limit, stick to it as close as possible
// - Setup Patreon
//
// Stream Schedule (Actual Topics):
// - ?? follow potential list when don't have schedule set ??
//
// Stream Topics (Potential Topics):
// - OSX Developer Environment
// - Linux Developer Environment (Debian apt-get, Arch pacman)
// - Win10 Developer Environment
// - Virtual Machine, QEMU, Bochs, VirtualBox (VMWare,Parallels,Docker,Vagrant)
// - Bootloader v1
//     - real mode
//     - print hello world
//     - simple 'shell' with couple commands
// - NASM assmebler (Intel Syntax)
// - Inline Assembly (AT&T Syntax)
// - Bootloaders v2 (GRUB, MultiBoot)
//     - Show how GRUB2 bootloader works (TranbyOS 0.1)
//     - Setting up GRUB-based kernel
//     - Set up GRUB Long Mode Kernel
// - Interrupts (IRQs, ISRs, GDT, IDT, etc)
// - Processor Modes (Real, Protected, Long)
// - Bootloader v3 (Protected Mode)
//     - Show how most of kernel from v2 is the same
//      - no more segments, flat addressing
// - Bootloader v4 (Long Mode)
//      - differences with 64-bit mode?
//      - show going back and forth from Real<-->Protected<-->Long
// - PIO ATA Hard Disk Read/Write
// - DMA ATA Hard Disk Read/Write
//
// - Process Structure
// - Memory Manager
// - Memory Paging and Page Table
// - Caching and TLB
// - CDROM Read/Write??
// - x86, x86_64, Arm64
//
//
// Ideas:
// - allow building and run on
//
//
// References:
//
// - Cryptographically Secure Pseudo-RNG (CSPNG):
//      https://en.wikipedia.org/wiki/Cryptographically_secure_pseudorandom_number_generator
// - arc4random uniform distribution:
//      http://www.openbsd.org/cgi-bin/man.cgi/OpenBSD-current/man3/arc4random.3?query=arc4random&sec=3
//
//
// TODO(steve):
// - Decide on Kernel Name, OS Name, codenames?
//      ASCII ART:
//          http://chris.com/ascii/index.php
//          http://www.patorjk.com/software/taag/#p=testall&f=Lil%20Devil&t=TranbyOS
// - Linux Kernel v0.1:
//      https://www.kernel.org/pub/linux/kernel/Historic/
// - Switch Processing Modes: Real <--> Protected <--> Long
//      http://www.codeproject.com/Articles/45788/The-Real-Protected-Long-mode-assembly-tutorial-for
// - CPUID:
//      https://github.com/rythie/CPUID-in-Assembly
// - Processor TLB Structure
// - Context Switch Capability
// - Fork Capability
// - Memory Allocator (malloc-like, free store, index forward and circle around, over time bring all blocks to the latest HEAD)
//   - see the video somewhere on SSD's not like erase, but write okay
// - Simple Shell (trash - TRAnby SHell)
//
//
// TODO(Future):
//
// - Build on Win10 Support
// -

// Regular Experession
// find: ^/////+///$
// repl: //////////////////////////////////////////////////////////////////
//
// swap params (simple)
// find: \((\w+\s+\w+), (\w+\s+\w+)\)
// repl: \($2, $1\)
//
// # Completed

// TODO:
// - https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_headers.html

// Require libc
// #include <stdint.h>

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
    #error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
    // #error "This tutorial needs to be compiled with a i386-elf compiler"
#endif

#include <system.h>
#include <systemcpp.h>
#include <multiboot.h>

//////////////////////////////

output_writer TRACE_WRITER = serial_write_b;

u32 initial_esp;

/////////////////////////////

void kassert_fail(c_str assertion, c_str file, unsigned int line, c_str func, c_str msg)
{
    for(int i=0; i<2; ++i) {
        output_writer writer = i == 0 ? serial_write_b : kputch;
        kwritef(writer, "[ASSERT]: %d: %s: %s: %s\n[%s]", line, file, func, msg, assertion);
    }
}

void* kmemcpy(void* restrict dest, const void* restrict src, size_t n)
{
    asm volatile("rep movsb"
                 : "=c"((i32){0})
                 : "D"(dest), "S"(src), "c"(n)
                 : "flags", "memory");
    return dest;
}

void* kmemset(void* dest, int c, size_t n)
{
    asm volatile("rep stosb"
                 : "=c"((int){0})
                 : "D"(dest), "a"(c), "c"(n)
                 : "flags", "memory");
    return dest;
}

size_t kmemcmp(const void* vl, const void* vr, size_t n)
{
    const u8* l = vl;
    const u8* r = vr;
    for (; n && *l == *r; n--, l++, r++);
    return n ? *l-*r : 0;
}

void* kmemchr(const void * src, int c, size_t n)
{
    const unsigned char * s = src;
    c = (unsigned char)c;
    // TODO: clarify code

    for (; ((uintptr_t)s & (ALIGN - 1)) && n && *s != c; s++, n--);
    if (n && *s != c) {
        const size_t * w;
        size_t k = ONES * c;
        for (w = (const void *)s; n >= sizeof(size_t) && !HASZERO(*w^k); w++, n -= sizeof(size_t));
        for (s = (const void *)w; n && *s != c; s++, n--);
    }
    return n ? (void *)s : 0;
}

void* kmemrchr(const void * m, int c, size_t n)
{
    const unsigned char * s = m;
    c = (unsigned char)c;
    while (n--) {
        if (s[n] == c) {
            return (void*)(s+n);
        }
    }
    return 0;
}

void* kmemmove(void * dest, const void * src, size_t n)
{
    char * d = dest;
    const char * s = src;

    if (d==s) {
        return d;
    }

    if (s+n <= d || d+n <= s) {
        return kmemcpy(d, s, n);
    }

    if (d<s) {
        if ((uintptr_t)s % sizeof(size_t) == (uintptr_t)d % sizeof(size_t)) {
            while ((uintptr_t)d % sizeof(size_t)) {
                if (!n--) {
                    return dest;
                }
                *d++ = *s++;
            }
            for (; n >= sizeof(size_t); n -= sizeof(size_t), d += sizeof(size_t), s += sizeof(size_t)) {
                *(size_t *)d = *(size_t *)s;
            }
        }
        for (; n; n--) {
            *d++ = *s++;
        }
    } else {
        if ((uintptr_t)s % sizeof(size_t) == (uintptr_t)d % sizeof(size_t)) {
            while ((uintptr_t)(d+n) % sizeof(size_t)) {
                if (!n--) {
                    return dest;
                }
                d[n] = s[n];
            }
            while (n >= sizeof(size_t)) {
                n -= sizeof(size_t);
                *(size_t *)(d+n) = *(size_t *)(s+n);
            }
        }
        while (n) {
            n--;
            d[n] = s[n];
        }
    }
    
    return dest;
}

////////////////////////////////

// memcpy - copy n bytes from src to dest
u8* kmemcpyb(u8* dest, const u8* src, u32 count)
{
    const u8* sp = (const u8*)src;
    u8* dp = (u8*)dest;
    for(; count != 0; count--)
        *dp++ = *sp++;
    return dest;
}

u8* kmemsetb(u8* dest, u8 val, u32 count)
{
    u8* temp = dest;
    while(count != 0) {
        *temp++ = val; --count;
    }
    return dest;
}

u16* kmemsetw(u16* dest, u16 val, u32 count)
{
    u16* temp = (u16*)dest;
    while(count != 0) {
        *temp++ = val; --count;
    }
    return dest;
}

// strlen - gets the length of a c-string
u32 kstrlen(c_str str)
{
    u32 retval = 0;
    while(*str != 0) {
        ++retval; ++str;
    }
    return retval;
}

// RAND_MAX assumed to be 32767
static u32 _next_rand = 1;
u32 krand(void)
{
    _next_rand = _next_rand * 1103515245 + 12345;
    return (_next_rand/65536) % 32768;
}

void srand(u32 seed)
{
    _next_rand = seed;
}

void wait_any_key() {
    set_text_color(COLOR_LIGHT_MAGENTA, COLOR_BLACK);

    // TODO: clear_key_buffer();
    kputs("Clearing Key Buffer\n");
    while(keyboard_read_next()) {
        kputch('.');
    }

    kputs("Waiting for Key\n");
    kputs("Press Any Key!");
    kgetch();

    set_text_color(COLOR_WHITE, COLOR_BLACK);

    kputs("\n");
    kputs("key pressed\n");
}

void display_banner()
{
    set_text_color(COLOR_GREEN, COLOR_BLACK);

    /// http://www.patorjk.com/software/taag/#p=testall&f=Lil%20Devil&t=TranbyOS
    ///
    kputs("                                                                            \n");
    kputs(" _/_/_/_/_/                            _/                   _/_/     _/_/_/ \n");
    kputs("    _/     _/  _/_/   _/_/_/ _/_/_/   _/_/_/    _/    _/ _/    _/ _/        \n");
    kputs("   _/     _/_/     _/    _/ _/    _/ _/    _/  _/    _/ _/    _/   _/_/     \n");
    kputs("  _/     _/       _/    _/ _/    _/ _/    _/  _/    _/ _/    _/       _/    \n");
    kputs(" _/     _/         _/_/_/ _/    _/ _/_/_/      _/_/_/   _/_/   _/_/_/       \n");
    kputs("                                                 _/                         \n");
    kputs("                                            _/_/                            \n");
    kputs("                                                                            \n");

    //kputs("\n-= Tranby OS =-\n");

    //set_text_color(COLOR_WHITE, COLOR_BLACK);

    kputs("Steve Tranby (stevetranby@gmail.com)\n");
    kputs("http://stevetranby.com/\n");
    kputs("http://github.com/stevetranby/tranbyos\n");
    kputs("http://osdev.org/\n");
    kputs("\n");
    kputs("This operating system is a test bed for experimenting and learning how to\n");
    kputs("write an operating system kernel, drivers, and possibly more.\n");
    kputs("\n");
}

#define my_test(x) _Generic((x), long double: my_testl, \
                                 default: my_testi, \
                                 float: my_testf)(x)

void my_testi(int i) { UNUSED_PARAM(i); kputs("int\n"); }
void my_testf(float f) { UNUSED_PARAM(f); kputs("float\n"); }
void my_testl(long double l) { UNUSED_PARAM(l); kputs("long double\n"); }


//#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

internal vbe_controller_info *vbe_ctrl;
internal vbe_mode_info *vbe;
internal uint8_t *mem;

// 24bpp
internal void draw_pixel(u32 x, u32 y, u32 color)
{
    u32 pos = y * vbe->LinBytesPerScanLine + x * (vbe->bpp / 8);
    // Assuming 24-bit color mode
    mem[pos] = color & 0xFF;
    mem[pos + 1] = (color >> 8) & 0xFF;
    mem[pos + 2] = (color >> 16) & 0xFF;
    //trace("draw pixel {%d, %d} [%d] w/color %x\n    ", x, y, pos, color);
}

internal void draw_rectangle(u32 x, u32 y, u32 width, u32 height, uint32_t color)
{
    trace("draw rect {%d, %d, %d, %d} w/color %x\n", x, y, width, height, color);
    for (u32 i = x; i < x + width; ++i)
    {
        for (u32 j = y; j < y + height; ++j)
        {
            draw_pixel(i, j, color);
        }
    }
}
