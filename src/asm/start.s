
;**********************************
; Author: Steve Tranby
; Using NASM, GCC (for now), ELF
; TranbyOS Boot and Assembly Hooks
; GRUB MultiBoot Loader 
;
; Ref: [TODO: osdev wiki link]
;
; Assembly Info
; https://en.wikibooks.org/wiki/X86_Assembly
; https://en.wikibooks.org/wiki/X86_Disassembly/Calling_Conventions
; https://en.wikibooks.org/wiki/X86_Assembly/NASM_Syntax
;**********************************

; Utilize 32-bit x86
[BITS 32]

extern _text_start
extern _text_end
extern _bss_start
extern _bss_end
extern _data_start
extern _data_end

extern kmain
extern gp                 
extern idtp               
extern fault_handler      
extern irq_handler        
extern test_user_function

global _entry
global gdt_flush	    
global tss_flush	    
global idt_load
global sys_stack_bottom
global sys_stack_top
global sys_heap_bottom
global sys_heap_top

section .text

_entry:
    mov esp, sys_stack_top		; This points the stack to our new stack area
    jmp stublet

; This part MUST be 4byte aligned, so we solve that issue using 'ALIGN 4'
section .multiboot
align 4
_mboot:
    ; Multiboot macros to make a few lines later more readable
    MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002

    MULTIBOOT_PAGE_ALIGN	equ 1<<0
    MULTIBOOT_MEMORY_INFO	equ 1<<1
    MULTIBOOT_VIDEO_INFO    equ 1<<2
    MULTIBOOT_AOUT_KLUDGE	equ 1<<16
    MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_INFO | MULTIBOOT_AOUT_KLUDGE
    MULTIBOOT_CHECKSUM		equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

    ; This is the GRUB Multiboot header. A boot signature
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM
    
    ; AOUT kludge - must be physical addresses. Make a note of these:
    ; The linker script fills in the data for these ones!
    dd _mboot            ; header
    dd _text_start      ; load addr
    dd _bss_start       ; load end, bss start
    dd _bss_end         ; bss end
    dd _entry           ; entry

    ; Request linear graphics mode
    dd 0        ; request mode type
    dd 0        ; request width
    dd 0        ; request height
    dd 32

section .text

; call our main() function 
stublet:
    push esp    ; pass in the kernel stack address
	push eax    ; contains 0x2BADB002 (magic)
	push ebx    ; header physical address
    cli
	call kmain
    cli

halt:
    hlt
    jmp $