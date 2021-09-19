; Utilize 32-bit x86
[BITS 16]

org 0x7c00    ;BIOS loaded at 0x7c00

bits 16       ; We are still in 16 bit real mode

Start:
    cli      ; clear all interrupts
    hlt      ; 