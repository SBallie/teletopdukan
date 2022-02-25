
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
register char __res; \
__asm__("push %%fs;mov %%ax,%%fs;movb %%fs:%2,%%al;pop %%fs" \
    :"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})

#define get_seg_long(seg,addr) ({ \
register u32 __res; \
__asm__("push %%fs;mov %%ax,%%fs;movl %%fs:%2,%%eax;pop %%fs" \
    :"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})
/*
#define str(n) \
__asm__("str %%ax\n\t" \
    "subl %2,%%eax\n\t" \
    "shrl $4,%%eax" \
    :"=a" (n) \
    :"a" (0),"i" (FIRST_TSS_ENTRY<<3))
*/
#define _fs() ({ \
register unsigned short __res; \
__asm__("mov %%fs,%%ax":"=a" (__res):); \
__res;})

internal void die(char * str, long esp_ptr, long nr)
{
    long * esp = (long *) esp_ptr;
    int i;

    output_writer writer = serial_write_b;

    kwritef(writer, "%s: %x\n\r", str, nr & 0xffff);
    kwritef(writer, "EIP:\t%x:%p\nEFLAGS:\t%p\nESP:\t%x:%p\n", esp[1], esp[0], esp[2], esp[4], esp[3]);
    kwritef(writer, "fs: %x\n", _fs());
    //kwritef(writer, "base: %p, limit: %p\n", get_base(current->ldt[1]), get_limit(0x17));
    if (esp[4] == 0x17) {
        kwritef(writer, "Stack: ");
        for (i=0; i<4; i++)
            kwritef(writer, "%p ",get_seg_long(0x17,i+(long *)esp[3]));
        kwritef(writer, "\n");
    }
    //str(i);
    //kwritef(writer, "Pid: %d, process nr: %d\n\r",current->pid,0xffff & i);
    for(i=0;i<10;i++)
        kwritef(writer, "%x ",0xff & get_seg_byte(esp[1],(i+(char *)esp[0])));
    kwritef(writer, "\n\r");
    // TODO: do_exit(11);        /* play segment exception */
}

//////////////////////////////////////////////////////////////////
// ISRS

/// Setup the IDT with to map interrupts to correct handler
void isrs_install()
{
    // TODO: use an "unused" stub/handler for those not used??
    // TODO: code generator or move this into NASM where we have repeat macro

#define IDT_GATE_INTERRUPT  0x8E // (dpl protection level max - ring = 0, Gate Type = 0xE)
#define IDT_GATE_TRAP       0x8F // (dpl protection level max - ring = 0, Gate Type = 0xF)
#define IDT_GATE_SYSTEM     0xEF // (dpl protection level max - ring = 3, Gate Type = 0xF)

    idt_set_gate(0,  (u32)isr0,  0x08, IDT_GATE_TRAP);
    idt_set_gate(1,  (u32)isr1,  0x08, IDT_GATE_TRAP);
    idt_set_gate(2,  (u32)isr2,  0x08, IDT_GATE_TRAP);
    idt_set_gate(3,  (u32)isr3,  0x08, IDT_GATE_SYSTEM);
    idt_set_gate(4,  (u32)isr4,  0x08, IDT_GATE_SYSTEM);
    idt_set_gate(5,  (u32)isr5,  0x08, IDT_GATE_SYSTEM);
    idt_set_gate(6,  (u32)isr6,  0x08, IDT_GATE_TRAP);
    idt_set_gate(7,  (u32)isr7,  0x08, IDT_GATE_TRAP);
    idt_set_gate(8,  (u32)isr8,  0x08, IDT_GATE_TRAP);
    idt_set_gate(9,  (u32)isr9,  0x08, IDT_GATE_TRAP);
    idt_set_gate(10, (u32)isr10, 0x08, IDT_GATE_TRAP);
    idt_set_gate(11, (u32)isr11, 0x08, IDT_GATE_TRAP);
    idt_set_gate(12, (u32)isr12, 0x08, IDT_GATE_TRAP);
    idt_set_gate(13, (u32)isr13, 0x08, IDT_GATE_TRAP);
    idt_set_gate(14, (u32)isr14, 0x08, IDT_GATE_TRAP);
    idt_set_gate(15, (u32)isr15, 0x08, IDT_GATE_TRAP);
    idt_set_gate(16, (u32)isr16, 0x08, IDT_GATE_TRAP);
    idt_set_gate(17, (u32)isr17, 0x08, IDT_GATE_TRAP);
    idt_set_gate(18, (u32)isr18, 0x08, IDT_GATE_TRAP);
    idt_set_gate(19, (u32)isr19, 0x08, IDT_GATE_TRAP);
    idt_set_gate(20, (u32)isr20, 0x08, IDT_GATE_TRAP);
    idt_set_gate(21, (u32)isr21, 0x08, IDT_GATE_TRAP);
    idt_set_gate(22, (u32)isr22, 0x08, IDT_GATE_TRAP);
    idt_set_gate(23, (u32)isr23, 0x08, IDT_GATE_TRAP);
    idt_set_gate(24, (u32)isr24, 0x08, IDT_GATE_TRAP);
    idt_set_gate(25, (u32)isr25, 0x08, IDT_GATE_TRAP);
    idt_set_gate(26, (u32)isr26, 0x08, IDT_GATE_TRAP);
    idt_set_gate(27, (u32)isr27, 0x08, IDT_GATE_TRAP);
    idt_set_gate(28, (u32)isr28, 0x08, IDT_GATE_TRAP);
    idt_set_gate(29, (u32)isr29, 0x08, IDT_GATE_TRAP);
    idt_set_gate(30, (u32)isr30, 0x08, IDT_GATE_TRAP);
    idt_set_gate(31, (u32)isr31, 0x08, IDT_GATE_TRAP);

    // Remap IRQs from 0 and 8 to => 32 and 40
    irq_remap(0x20, 0x28);

    // mapping IRQ handlers above the default CPU ones
    idt_set_gate(32, (u32)irq0, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(33, (u32)irq1, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(34, (u32)irq2, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(35, (u32)irq3, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(36, (u32)irq4, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(37, (u32)irq5, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(38, (u32)irq6, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(39, (u32)irq7, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(40, (u32)irq8, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(41, (u32)irq9, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(42, (u32)irq10, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(43, (u32)irq11, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(44, (u32)irq12, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(45, (u32)irq13, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(46, (u32)irq14, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(47, (u32)irq15, 0x08, IDT_GATE_INTERRUPT);

    // TODO: other above 47, some use 128 for sys calls?
    //idt_set_gate(47, (u32)irq127, 0x08, );
}

/// CPU Exception Message
///
/// http://wiki.osdev.org/Exceptions
///
///
c_str exception_messages[] =
{
    // 0x00 - 0x09
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",