
#include <system.h>

/* Defines a GDT entry. We say packed, because it prevents the
*  compiler from doing things that it thinks is best: Prevent
*  compiler "optimization" by packing */
typedef struct PACKED
{
    u16 limit_low;
    u16 base_low;
    u8 base_middle;
    u8 access;
    u8 granularity;
    u8 base_high;
} gdt_entry;

/* Special pointer which includes the limit: The max bytes
*  taken up by the GDT, minus 1. Again, this NEEDS to be packed */
typedef struct PACKED
{
    u16 limit;
    gdt_entry* base;
} gdt_ptr;

typedef struct PACKED {
    u32    prev_tss;
    u32    esp0;
    u32    ss0;
    u32    esp1;
    u32    ss1;
    u32    esp2;
    u32    ss2;
    u32    cr3;
    u32    eip;
    u32    eflags;
    u32    eax;
    u32    ecx;
    u32    edx;
    u32    ebx;
    u32    esp;
    u32    ebp;
    u32    esi;
    u32    edi;
    u32    es;
    u32    cs;
    u32    ss;
    u32    ds;
    u32    fs;
    u32    gs;
    u32    ldt;
    u16    trap;
    u16    iomap_base;
} tss_entry;


// This will be a function in start.asm.
// We use this to properly reload the new segment registers
extern void gdt_flush();
extern void tss_flush();

/// Global Descriptor Table (GDT) entries
// TODO: may need to lock around access to these?
gdt_entry gdt[6];
gdt_ptr gp;
tss_entry tss;

/* Setup a descriptor in the Global Descriptor Table */
void gdt_set_gate(u32 num, u32 base, u32 limit, u8 access, u8 gran)
{
    // Setup the descriptor base address 
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    // Setup the descriptor limits 
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    // Finally, set up the granularity and access flags 
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0);

/* Should be called by main. This will setup the special GDT
*  pointer, set up the first 3 entries in our GDT, and then
*  finally call gdt_flush() in our assembler file in order
*  to tell the processor where the new GDT is and update the
*  new segment registers */
void gdt_install()
{
    // Setup the GDT pointer and limit
    gp.limit = sizeof(gdt_entry) * 6 - 1;