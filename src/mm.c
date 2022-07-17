
#include <system.h>

// NOTE: the linker symbol variables have their addresses defined
//       value stored as the pointer is data found at symbol address
// ex: 0x00100000 (address of _text_start)
//     0x1badb002 (data @ _text_start cast as void*) <- multiboot header
extern intptr_t _text_start;
extern intptr_t _text_end;
extern intptr_t _data_start;
extern intptr_t _data_end;
extern intptr_t _bss_start;
extern intptr_t _bss_end;
// these
extern intptr_t sys_stack_bottom;
extern intptr_t sys_stack_top;
extern intptr_t sys_heap_bottom;
extern intptr_t sys_heap_top;

// Define max blocks allowed to be allocated
#define MAX_BLOCKS 2048

struct block;
struct freeblock;
typedef struct block block_t;
typedef struct freeblock freeblock_t;

struct block {
	char *base;
	block_t *next;
	u32 len;
};

struct freeblock {
	char *base;
	freeblock_t *next;
	u32 len;
};

// NOTE: current kernel blocks are byte-sized
u32 blocks_used[MAX_BLOCKS];

//static void *brkval;
u8* heap_ptr;
u8* free_ptr;
//static block_t *freelist;

// Initialize the Memory Manager
void init_mm()
{
    kmemset(blocks_used, 0, MAX_BLOCKS);
    // Let's start simple and create just a standard heap
    heap_ptr = (u8*)&sys_heap_bottom + 1;
    free_ptr = heap_ptr;

    trace("heap_ptr = %x\n", heap_ptr);
    trace("free_ptr = %x\n", free_ptr);
    {
        // TEST MAGIC
        u8* ptr = kmalloc_b(5);
        trace("ptr = %p\n", ptr);
        ptr[0] = 'H';
        ptr[1] = 'E';
        ptr[2] = 'A';
        ptr[3] = 'P';
        ptr[4] = '\0';
    }
    {
        // TEST MAGIC
        u8* ptr = kmalloc_b(5);
        trace("ptr = %p\n", ptr);
        ptr[0] = 'H';
        ptr[1] = 'E';
        ptr[2] = 'A';
        ptr[3] = 'P';
        ptr[4] = '\0';
    }
}

void print_heap_magic()
{
    trace("heap magic\n");

    // NOTE: the linker symbol variables have their addresses defined
    //       value stored as the pointer is data found at symbol address
    // ex: 0x00100000 (address of _text_start)
    //     0x1badb002 (data @ _text_start cast as void*) <- multiboot header
    trace("Text Section: %p | %p\n", &_text_start, &_text_end);
    trace("Data Section: %p | %p\n", &_data_start, &_data_end);
    trace("BSS Section:  %p | %p\n", &_bss_start, &_bss_end);
    trace("STACK:        %p | %p\n", &sys_stack_bottom, &sys_stack_top);
    trace("HEAP:         %p | %p\n", &sys_heap_bottom, &sys_heap_top);
    trace("Heap Magic String: %p '%s'\n", heap_ptr, heap_ptr);
    trace("Free Ptr:          %p\n", free_ptr);

}

void print_heap_bytes(u32 n)
{
    trace( "print_heap_bytes:\n\n");
    int i = 0;
    for(u32* ptr = (u32*)heap_ptr; i < n; ++i, ++ptr) {
        kserialf( "%x", *ptr);
        if(0 == (i+1) % 8) kserialf( "\n");
        else kserialf( " ");
    }
    kserialf("\n");
}

// TODO: a lot of potential places to macro or pre-proc out the excess
// TODO: do we want this? it's a nice handy shortcut
#define FOR(n) for(int i=0; i<10;)

void print_blocks_avail()
{
    trace("blocks available:\n\n");
    for(int i = 0; i < MAX_BLOCKS; ++i)
    {
        kserialf( "%d", blocks_used[i]);
        if(0 == (i+1) % 60) kserialf("\n");
        else kserialf( ",");
    }
    kserialf("\n");
}

// TODO: allocate extra block on each side and fill with debug markers
// TODO: make block size larger (multiple sizes)
// Byte Allocator for Heap
u8* kmalloc_b(u32 nblks)
{
    u32 offset = free_ptr - heap_ptr;

    // TODO: wrap around search and try to find contiguous space in freed blocks
    //       if wrapping need to check blocks_used[] to make sure it's free
    if((u32)offset + nblks > MAX_BLOCKS) {
        trace("TODO: need to have wrap around malloc\n");
		return NULL;
    }

    // mark used
    trace( "blocks_used %p, %d, %d, %d\n", blocks_used + offset, offset, nblks, nblks);
    kmemset(blocks_used + offset, nblks, nblks);
