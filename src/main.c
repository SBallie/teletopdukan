
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