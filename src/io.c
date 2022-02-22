
#include <system.h>

//static inline u8 inb_dummy(u16 port)
//{
//    u8 ret;
//    asm volatile ( "inb %[port], %[ret]"
//                  : [ret] "=a"(ret)
//                  : [port] "Nd"(port) );
//    return ret;
//}

// TODO: maybe just move these into .asm? why bother just because
//       we want everything in C?


/// Read from IO port (8,16,32-bit value)
u8 inb (u16 _port)
{
    u8 rv;
    asm volatile ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

u16 inw(u16 _port)
{
    u16 rv;
    asm volatile ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

u32 inl(u16 _port)
{
    u32 rv;
    asm volatile ("inl %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

/// Write to IO port (8,16,32-bit value)
void outb(u16 _port, u8 _data)
{
    asm volatile ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

void outw(u16 _port, u16 _data)
{
    asm volatile ("outw %1, %0" : : "dN" (_port), "a" (_data));
}

void outd(u16 _port, u32 _data)
{
    asm volatile ("outl %1, %0" : : "dN" (_port), "a" (_data));
}

///*
// * memsetw
// * Set `count` shorts to `val`.
// */
//unsigned short * memsetw(unsigned short * dest, unsigned short val, int count) {
//    int i = 0;
//    for ( ; i < count; ++i ) {
//        dest[i] = val;
//    }
//    return dest;
//}
//
//uint32_t __attribute__ ((pure)) krand(void) {
//    static uint32_t x = 123456789;
//    static uint32_t y = 362436069;
//    static uint32_t z = 521288629;
//    static uint32_t w = 88675123;
//
//    uint32_t t;
//
//    t = x ^ (x << 11);