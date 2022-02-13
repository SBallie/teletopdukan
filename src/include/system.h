
#pragma once

#include <stdbool.h> /* C doesn't have booleans by default. */
#include <stddef.h>
#include <float.h>
#include <limits.h>
#include <iso646.h>
//#include <stdtypes.h>
#include <stdarg.h>
#include <stdint.h>

//////////////////////////////////////////////////////////////////
// Types

// Data Types
// TODO: make sure to #ifdef or include in platform-specific header
// TODO: 's' instead of 'i' prefix?
typedef uint8_t    u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef char       i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef uint8_t    b8;
typedef uint32_t  b32;
typedef int64_t   i64;
typedef uint64_t  u64;

typedef float              real32;
typedef double             real64;
typedef float                 f32;
typedef double                f64;
typedef const char*         c_str;
typedef char*           c_str_mut;

//////////////////////////////////////////////////////////////////
// Variable arg functions (t, ....)

// NOTE: uses <stdarg.h> and <stddef.h> by default
#ifndef va_start
typedef __builtin_va_list va_list;
#define va_start(ap,last) __builtin_va_start(ap, last)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap,type) __builtin_va_arg(ap,type)
#define va_copy(dest, src) __builtin_va_copy(dest,src)
#endif

#ifndef NULL
#define NULL ((void *)0UL)
#endif

//////////////////////////////////////////////////////////////////
// Macro Playground

/* The token pasting abuse that makes this possible */
#define JOIN_INTERNAL( a, b ) a##b
#define JOIN( a, b ) JOIN_INTERNAL( a, b )
#define REPEAT_x0_x9(a,x) a(x##0); a(x##1); a(x##2); a(x##3); a(x##4); a(x##5); a(x##6); a(x##7); a(x##8); a(x##9);

//////////////////////////////////////////////////////////////////
// Defined Constants

// make it more clear why we're using static
#define global   static
#define internal static

#define UNUSED_PARAM(x) ((void)(x))
#define UNUSED_VAR(x)   ((void)(x))

#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(X) (((X)-ONES) & ~(X) & HIGHS)

//#define BITOP(A, B, OP) ((A)[(size_t)(B)/(8*sizeof *(A))] OP (size_t)1<<((size_t)(B)%(8*sizeof *(A))))

#define BIT(data,bit) (data & (1 << bit))

#define KILO (1024)            // 2^10
#define MEGA (1024*1024)       // 2^20
#define GIGA (1024*1024*1024)  // 2^30


#define MAX(a,b) (a < b ? b : a)
#define MIN(a,b) (a < b ? a : b)
#define CLAMP(x, low, high) MIN(MAX(x,low), high)

//////////////////////////////////////////////////////////////////
// Macros

// TODO: only need to make sure work on other compilers we want to use for building the kernel, user level can be built with other compilers

// Volatile - don't move instructions (no optimization)
#define asm             __asm__
#define volatile    __volatile__
#define PACKED      __attribute__((packed))

#define sti()   asm volatile ("sti");
#define cli()   asm volatile ("cli");
#define nop()   asm volatile ("nop");
#define iret()  asm volatile ("iret");
#define pusha() asm volatile ("pusha");
#define popa()  asm volatile ("popa");

//------------------
// Error Handling


//////////////////////
// Assertions and Errors

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define	CONCAT(x,y)   x ## y
#define	STRINGIFY(x)  #x
extern void kassert_fail(c_str assertion, c_str file, unsigned int line, c_str func, c_str msg);
#define ASSERT(expr,msg) ((void) ((expr) || (kassert_fail(STRINGIFY(expr), __FILE__, __LINE__, __func__, msg), 0)))

//////////////////////////////////////////////////////////////////
// Utilities and Common

// TODO: determine how to handle the desire for restrict in cpp code
#if __cplusplus
#define restrict
#endif
extern void* kmemcpy(void* restrict dest, const void* restrict src, size_t n);
extern void* kmemset(void* dest, int c, size_t n);
extern size_t   kmemcmp(const void* vl, const void* vr, size_t n);
extern void* kmemchr(const void* src, int c, size_t n);
extern void* kmemrchr(const void * m, int c, size_t n);
extern void* kmemmove(void * dest, const void * src, size_t n);

extern u8*  kmemcpyb(u8* dest, const u8* src, u32 count);
extern u8*  kmemsetb(u8* dest, u8 val, u32 count);
extern u16* kmemsetw(u16* dest, u16 val, u32 count);
extern u32  kstrlen(c_str str);
extern u32  krand(void);
extern void ksrand(u32 seed);

//////////////////////////////////////////////////////////////////
// Real Time & Clock and Timers


// TODO: get a calendar library
// Clock "Real" Time (from battery-backed CMOS)
// second: 0-59, minute: 0-59, day: 0-31, month: 0-12, year: yyyy
typedef struct
{
    u8 second;
    u8 minute;
    u8 hour;
    u8 day;
    u8 month;
    u16 year;
} rtc_time;

extern void timer_install();
extern u32 timer_ticks();
extern u32 timer_seconds();
extern void delay_ticks(i32 ticks);
extern void delay_ms(u32 ms);
extern void delay_s(u32 s);
extern rtc_time read_rtc();

//////////////////////////////////////////////////////////////////
// Filesystem (FS)

/// FS Block -
typedef struct {
    u32 a;
    u8 b;
    i16 add;
} fs_block;


/// FS Master Table
/// - the top level table storing pointers to block paging tables, etc
typedef struct {

} fs_master_table;

//////////////////////////////////////////////////////////////////
// Screen (Terminal, VGA, VESA)

// TODO: const u8
#define COLOR_BLACK         0x00
#define COLOR_BLUE          0x01
#define COLOR_GREEN         0x02
#define COLOR_CYAN          0x03
#define COLOR_RED           0x04
#define COLOR_MAGENTA       0x05
#define COLOR_BROWN         0x06
#define COLOR_LIGHT_GREY    0x07
#define COLOR_DARK_GREY     0x08
#define COLOR_LIGHT_BLUE    0x09
#define COLOR_LIGHT_GREEN   0x0a
#define COLOR_LIGHT_CYAN    0x0b
#define COLOR_LIGHT_RED     0x0c
#define COLOR_LIGHT_MAGENTA 0x0d
#define COLOR_YELLOW        0x0e
#define COLOR_WHITE         0x0f

// TODO: use Apple-style (maybe others) consts ????
enum {
    kColor_Black,
};

extern void cls();
extern u8 kgetch();
extern void kputch(u8 c);
extern void kputs(c_str str);
extern void set_text_color(u8 forecolor, u8 backcolor);
extern void init_video();

// vga
extern u32 init_graph_vga(u32 width, u32 height, b32 chain4);
extern void plot_pixel(u32 x, u32 y, u8 color);
extern void line_fast(u32 x1, u32 y1, u32 x2, u32 y2, u8 color);
extern void polygon(u32 num_vertices,  u32 *vertices, u8 color);
extern void fillrect(u32 xoff, u32 yoff, u8 color);
extern void vga_tests();


//////////////////////////////////////////////////////////////////
// Device Input/Output

extern u8 inb(u16 _port);
extern void outb(u16 _port, u8 _data);
extern u16 inw(u16 _port);
extern void outw(u16 _port, u16 _data);
extern void print_port(u16 port);

// TODO: move elsewhere??? Inline Asm Funcs
//extern inline void cpuid(int code, u32* a, u32* d);
//extern inline u64 rdtsc();
//extern inline void lidt(void* base, u16 size);
//extern inline void invlpg(void* m);
//extern inline void wrmsr(u32 msr_id, u64 msr_value);
//extern inline u64 rdmsr(u32 msr_id);

//////////////////////////////////////////////////////////////////
// Memory, Page Faults, Page Tables

/// Global Descriptor Table (GDT)
/// - Defines the memory paging table map
extern void gdt_set_gate(u32 num, u32 base, u32 limit, u8 access, u8 gran);
extern void gdt_install();

// Memory Allocation
extern void init_mm();
extern void print_heap_magic();
extern void print_heap_bytes(u32 n);
extern void print_blocks_avail();

extern u8* kmalloc_b(u32 size);
extern void free_b(u8* addr);

extern void loadPageDirectory(u32* page_directory);
extern void enablePaging();
extern void init_page_directory();


//////////////////////////////////////////////////////////////////
// Multitasking (Tasks, TSS, etc)
// Paging

// TODO: can we use packed bitfield structs without issue?
// - do we want to? if not create helper funcs for bit twiddle with DEFINES
typedef struct PACKED
{
    u32 present:1;
    u32 readwrite:1;
    u32 accessRing3:1;
    u32 writeThrough:1;
    u32 cacheDisabled:1;
    u32 accessed:1;
    u32 _zero:1;
    u32 pageSize:1;
    u32 _ignored:1;
    u32 _unused:3;
    u32 address:20;
} page_directory_entry;

typedef struct PACKED
{
    u32 present:1;
    u32 readwrite:1;
    u32 accessRing3:1;
    u32 writeThrough:1;
    u32 cacheDisabled:1;
    u32 accessed:1;
    u32 dirty:1;
    u32 isGlobal:1;
    u32 _unused:3;
    u32 frameAddress:20;
} page_table_entry;

typedef struct PACKED
{
    page_table_entry pages[1024];
} page_table;

typedef struct PACKED
{
    page_table* tables[1024];
    u32 tablesPhysical[1024];
    u32 physicalAddress;
} page_directory;

#define PAGE_DIR_PRESENT         	0x00000001 // 1 - in physical memory
#define PAGE_DIR_READWRITE          0x00000002 // 1 - readwrite, 0 - readonly
#define PAGE_DIR_ACCESS_RING3       0x00000004 // 1 - access by ALL, 0 - only supervisor
#define PAGE_DIR_WRITE_THROUGH      0x00000008 // 1 - write-through, 0 - write-back
#define PAGE_DIR_CACHE_DISABLED     0x00000010 // 1 - page will NOT be cached
#define PAGE_DIR_ACCESSED_RECENTLY  0x00000020 // 1 - read/written recently, OS must clear
#define PAGE_DIR_ZERO_unused        0x00000040 // for OS use
#define PAGE_TABLE_DIRTY            0x00000040 // written to, OS must clear
#define PAGE_DIR_PAGE_SIZE_4M       0x00000080 // 1 - 4MB pages, 0 - 4KB pages
#define PAGE_TABLE_zero_unused      0x00000080 // for OS use
#define PAGE_DIR_ignored            0x00000100 // ignored ...
#define PAGE_TABLE_GLOBAL           0x00000100 // The Global, or 'G' above, flag, if set, prevents the TLB from updating the address in it's cache if CR3 is reset. Note, that the page global enable bit in CR4 must be set to enable this feature.
#define PAGE_DIR_unused             0x00000200 // for OS use
#define PAGE_DIR_unused1            0x00000400 // for OS use
#define PAGE_DIR_unused2            0x00000800 // for OS use
#define PAGE_DIR_ADDR_BASE          0x00001000 // Base bit of start of address bits
#define PAGE_DIR_ADDR_MASK          0xfffff000 // mask of all bits in address


#define KERNEL_STACK_SIZE 2048       // Use a 2kb kernel stack.

typedef uintptr_t vaddr;
typedef uintptr_t paddr;

// TODO: check out isr_stack_state for task switching from IRQ
typedef struct {
    // general (0,4,8,12)
    u32 eax, ebx, ecx, edx;
    // special (eax + 16,20,24,28,32)
    u32 esi, edi, esp, ebp, eip;
    // code segment
    //TODO: u32 cs;
    // flags (eax + 36)
    u32 eflags;
    // page directory (eax + 40)
    u32 cr3;
} TaskRegisters;

// NOTE: require struct w/ tag because typedef not defined yet
typedef struct Task {
    TaskRegisters regs;
    i32 pid;
    //u32 esp, ebp, eip;
    page_directory* pageDirectory;
    u32 kernel_stack;
    b32 isActive;
    struct Task* next;
} Task;

void switch_page_directory(page_directory* newDirectory);
page_table_entry* get_page(u32 address, int make, page_directory* dir);
void page_fault(TaskRegisters* regs);
page_directory* clone_directory(page_directory* src);

typedef void(*TaskHandler)();

/// Initialize the multitasking system and structures
extern void initTasking();
/// Kernel interface to switching task
extern void preemptCurrentTask();
extern void createTask(Task*, TaskHandler, u32, u32*);
/// Kernel impl for switching task
extern void switchTask(TaskRegisters* prev, TaskRegisters* next);
extern void switchTaskInterrupt(TaskRegisters* prev, TaskRegisters* next);

extern void initialise_tasking();
extern i32 fork();
extern void move_stack(void* newStackStart, u32 stackSize);
extern i32 getpid();
extern void switch_task();
extern void set_kernel_stack(uintptr_t stack);

// testing
extern void jump_usermode();
extern void k_preempt();
extern void k_preempt_kernel();
extern void k_doIt();

//////////////////////////////////////////////////////////////////
// STDOUT and friends

typedef void(*output_writer)(u8 a);

extern void writeInt(output_writer writer, i32 num);
extern void writeUInt(output_writer writer, u32 num);
extern void writeInt64(output_writer writer, i64 num);
extern void writeUInt64(output_writer writer, u64 num);
extern void writeAddr(output_writer writer, void* ptr);
extern void writeHex_b(output_writer writer, u8 num);
extern void writeHex_w(output_writer writer, u16 num);
extern void writeHex(output_writer writer, u32 num);
extern void writeHex_q(output_writer writer, u64 num);
extern void writeHexDigit(output_writer writer, u8 digit);
extern void writeBinary_b(output_writer writer, u8 num);
extern void writeBinary_w(output_writer writer, u16 num);
extern void writeBinary(output_writer writer, u32 num);
extern void writeChar(output_writer writer, u8 ch);

extern void printInt(i32 num);
extern void printHex(u32 num);
extern void printHex_w(u16 num);
extern void printHex_b(u8 num);
extern void printAddr(void* ptr);
extern void printBinary_b(u8 num);

/// serial communication for output through QEMU/BOCHS serial terminal interface view
extern void init_serial();
extern int serial_received();
extern char read_serial();
extern u32 is_transmit_empty();
extern void serial_write_b(u8 a);
extern void serial_write(c_str str);
extern void serial_writeInt(u32 num);
extern void serial_writeHex(u32 num);
extern void serial_writeHex_w(u16 num);
extern void serial_writeHex_b(u8 num);
extern void serial_writeBinary_b(u8 num);

/// formatted strings
extern void kwrites(output_writer writer, c_str text);
extern void kwritef(output_writer writer, c_str format, ...);
#define kprintf(fmt, ...) kwritef(kputch, fmt, ##__VA_ARGS__)
#define kserialf(fmt, ...) kwritef(serial_write_b, fmt, ##__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////
// User Input Devices

typedef struct {
    u32 magic;
    i16 x_difference;
    i16 y_difference;
    u16 buttons;
} mouse_device_packet;

#define MOUSE_MAGIC 0x57343
#define LEFT_CLICK 0x01
#define RIGHT_CLICK 0x02
#define MIDDLE_CLICK 0x04
#define MOUSE_SCROLL_DOWN 0x08
#define MOUSE_SCROLL_UP 0x10

extern i32 mouse_get_x();
extern i32 mouse_get_y();
extern u16 mouse_get_buttons();
extern i8 mouse_get_scrolling();

extern void ps2_install();

////////////////////////////////////////////////////////////////////////////
// Disks (Hard Drive, Floppy, CDRom)

#define SECTOR_BYTES       512
#define SECTOR_WORDS       256
#define SECTOR_DWORDS      128     

// could use if wanting more than one controller
#define IDE_PRIMARY        1
#define IDE_SECONDARY      0

// could use if want a programmed way to access ATA registers
#define IDE0_BASE          0x1F0
#define IDE1_BASE          0X170

#define HD0_IRQ            IRQ14
#define HD1_IRQ            IRQ15

#define HD_DATA            0x1f0  // Data port
#define HD_FEAT            0x1f1  // Features (write)
#define HD_ERR             0x1f1  // Error Info (read)
#define HD_SC              0x1f2  // Sector Count
#define HD_SN              0x1f3  // Sector Number (Low Byte of LBA - Partial Disk Sector Address)
#define HD_CL              0x1f4  // Cylinder low-byte (Mid Byte of LBA - Partial Disk Sector Address)
#define HD_CH              0x1f5  // Cylinder high-byte (High Byte of LBA - Partial Disk Sector Address)
#define HD_DH              0x1f6  // Drive select bit, 101DHHHH
#define HD_ST              0x1f7  // Status port (read)
#define HD_CMD             0x1f7  // Command port (write)
#define HD_ST_ALT          0x3f6  // Alternative Status
#define HD_DCR             0x3f6  // Device Control Register (Alternative Status)

#define HD1_DATA           0x170  // Data port
#define HD1_FEAT           0x171  // Features (write)
#define HD1_ERR            0x171  // Error Info (read)
#define HD1_SC             0x172  // Sector Count
#define HD1_SN             0x173  // Sector Number (Low Byte of LBA - Partial Disk Sector Address)
#define HD1_CL             0x174  // Cylinder low-byte (Mid Byte of LBA - Partial Disk Sector Address)
#define HD1_CH             0x175  // Cylinder high-byte (High Byte of LBA - Partial Disk Sector Address)
#define HD1_DH             0x176  // Drive select bit, 101DHHHH
#define HD1_ST             0x177  // Status port (read)
#define HD1_CMD            0x177  // Command port (write)
#define HD1_ST_ALT         0x3f7  // Alternative Status
#define HD1_DCR            0x3f7  // Device Control Register (Alternative Status)

#define HD_DCR_HOB          0x80  // SEt this to read back high-order byte of last LBA48 value sent to IO port.
#define HD_DCR_SRST         0x04  // Software Reset -- set this to reset all ATA drives on a bus, if one is misbehaving.
#define HD_DCR_NIEN         0x02  // Set this to stop the current device from sending interrupts.

// Bits for HD_STATUS
#define HD_ST_ERR           0x01  // Error flag (when set). Send new command to clear it (or nuke with Soft-Reset)
#define HD_ST_IDX           0x02  // ?
#define HD_ST_ECC           0x04  // corrected errors
#define HD_ST_DRQ           0x08  // Set when drive has PIO data to transfer, or is ready to accept PIO data.
#define HD_ST_SK            0x10  // Overlapped Mode Service Request (seek)
#define HD_ST_DFE           0x20  // Drive fault errors (does not set ERR!)
#define HD_ST_RDY           0x40  // Bit is clear when drive is spun down, or after an error. Set otherwise.
#define HD_ST_BSY           0x80  // drive is preparing to accept/send data -- wait until this bit clears. If it never clears, then do a soft-reset. When set other status bits are meaningless.

// Values for HD_CMD
#define HD_CMD_RESTORE      0x10  //
#define HD_CMD_READ         0x20  //
#define HD_CMD_WRITE        0x30  //
#define HD_CMD_VERIFY       0x40  //
#define HD_CMD_FORMAT       0x50  //
#define HD_CMD_INIT         0x60  //
#define HD_CMD_SEEK         0x70  //
#define HD_CMD_DIAGNOSE     0x90  //
#define HD_CMD_SPECIFY      0x91  //
#define HD_CMD_IDENTIFY     0xEC  //

// Bits for HD_ERROR
#define HD_ERR_MARK         0x01  //
#define HD_ERR_TRACK0       0x02  //
#define HD_ERR_ABORT        0x04  //
#define HD_ERR_ID           0x10  //
#define HD_ERR_ECC          0x40  //
#define HD_ERR_BBD          0x80  //

extern void ata_delay400ns(void);
extern void ata_wait_busy();
extern void ata_wait_drq();
extern void ata_wait_ready();
extern int ata_soft_reset(void);
extern int ata_pio_read_w(int controller, int slave, int sn, int sc, u16 *data);
extern int ata_pio_write_w(int controller, int slave, int sn, int sc, u16 *data);
extern int ata_controller_present(int controller);