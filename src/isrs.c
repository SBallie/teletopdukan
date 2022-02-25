
#include <system.h>

// Interrupts & CPU Exception Handling
// - http://wiki.osdev.org/Interrupts
// - http://wiki.osdev.org/Exceptions
// - https://en.wikipedia.org/wiki/Interrupt_descriptor_table

void disable_irq(int irq);
void disable_irq_nosync(int irq);
void enable_irq(int irq);

// Disable PIC if using APIC and IOAPIC
//mov al, 0xff
//out 0xa1, al
//out 0x21, al

////////////////////////////////////////////////////////////////////////////////////
/// IDT

#define MAX_INTERRUPTS 256

// Store 256 interrupt entries, 32 ISR, 16 IRQ, rest "unhandled interrupt" exception
idt_entry idt[MAX_INTERRUPTS];
idt_ptr   idtp;

/// Add IDT entry
void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags)
{
    // The interrupt routine's base address
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

// Install IDT
void idt_install()
{
    // Sets the special IDT pointer up, just like in 'gdt.c'
    idtp.limit = sizeof(idt_entry) * 256 - 1;
    idtp.base = (u32)&idt;

    // Clear out the entire IDT, initializing it to zeros
    kmemsetb((u8*)&idt, 0, sizeof(idt));

    // Add any new ISRs to the IDT here using idt_set_gate
    isrs_install();

    // Points the processor's internal register to the new IDT
    idt_load();
}

//////////////////////////////////////////


// TODO: linux v1 source

/*
 
#define get_seg_byte(seg,addr) ({ \
register char __res; \
__asm__("push %%fs;mov %%ax,%%fs;movb %%fs:%2,%%al;pop %%fs" \
:"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})

#define get_seg_long(seg,addr) ({ \
register u32 __res; \
__asm__("push %%fs;mov %%ax,%%fs;movl %%fs:%2,%%eax;pop %%fs" \
:"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})

#define _fs() ({ \
register unsigned short __res; \
__asm__("mov %%fs,%%ax":"=a" (__res):); \
__res;})

#define str(n) \
__asm__("str %%ax\n\t" \
"subl %2,%%eax\n\t" \
"shrl $4,%%eax" \
:"=a" (n) \
:"a" (0),"i" (FIRST_TSS_ENTRY<<3))

 */

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// From linux kernel 0.1

#define PAGE_ALIGN(n) ( ( (n) + 0xfff ) & 0xfffff000)
/*
#define _set_base(addr,base) \
__asm__("movw %%dx,%0\n\t" \
    "rorl $16,%%edx\n\t" \
    "movb %%dl,%1\n\t" \
    "movb %%dh,%2" \
    ::"m" (*((addr)+2)), \
      "m" (*((addr)+4)), \
      "m" (*((addr)+7)), \
      "d" (base) \
    :"dx")

#define _set_limit(addr,limit) \
__asm__("movw %%dx,%0\n\t" \
    "rorl $16,%%edx\n\t" \
    "movb %1,%%dh\n\t" \
    "andb $0xf0,%%dh\n\t" \
    "orb %%dh,%%dl\n\t" \
    "movb %%dl,%1" \
    ::"m" (*(addr)), \
      "m" (*((addr)+6)), \
      "d" (limit) \
    :"dx")
*/
// #define set_base(ldt,base) _set_base( ((char *)&(ldt)) , base )
// #define set_limit(ldt,limit) _set_limit( ((char *)&(ldt)) , (limit-1)>>12 )

#define _get_base(addr) ({\
u32 __base; \
__asm__("movb %3,%%dh\n\t" \
    "movb %2,%%dl\n\t" \
    "shll $16,%%edx\n\t" \
    "movw %1,%%dx" \
    :"=d" (__base) \
    :"m" (*((addr)+2)), \
     "m" (*((addr)+4)), \
     "m" (*((addr)+7))); \
__base;})

#define get_base(ldt) _get_base( ((char *)&(ldt)) )

#define get_limit(segment) ({ \
u32 __limit; \
__asm__("lsll %1,%0\n\tincl %0":"=r" (__limit):"r" (segment)); \
__limit;})

#define get_seg_byte(seg,addr) ({ \