;
; Save for later, or testing

[BITS 32]

extern test_user_function

;--------------------------------------------------------------------

; Load the index of our TSS structure - The index is
; 0x28, as it is the 5th selector and each is 8 bytes
; long, but we set the bottom two bits (making 0x2B)
; so that it has an RPL of 3, not zero.
; Load 0x2B into the task state register.

section text

global tss_flush
tss_flush:
    mov ax, 0x2B
    ltr ax
    ret

global jump_usermode
jump_usermode:
     mov ax, 0x23
     mov ds, ax
     mov es, ax
     mov fs, ax
     mov gs, ax      ;we don't need to worry about SS. it's handled by iret
     mov eax, esp
     push 0x23      ;user data segment with bottom 2 bits set for ring 3
     push eax       ;push our current stack just for the heck of it
     pushf          ;push the Eflags register
     push 0x1B      ;user data segment with bottom 2 bits set for ring 3
     push test_user_function
     iret           


global switchTask
switchTask:
                            ; arg1, arg2 (esp -= 8)
                            ; eip (esp -= 4)