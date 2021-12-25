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
    pusha                   ; ax,bx,cx,dx(16),sp,bp,si,di(16) (esp -= 32)
    pushf                   ; eflags (esp -= 4)
    
    mov eax, cr3                                         
    push eax                ; cr3 (esp -= 4)

    ; ESP  +  0 ,   4   ,  8 , 12 , 16 , 20 , 24 , 28 , 32 , 36 , 40 , 44 , 48
    ; Stack: cr3, eflags, edi, esi, ebp, esp, edx, ecx, ebx, eax, eip, arg1, arg2
                                               
    mov eax, [esp+44]       ; The first argument, where to save
     
    mov [eax+4], ebx        ;   
    mov [eax+8], ecx        ;   
    mov [eax+12], edx       ;   
    mov [eax+16], esi       ;   
    mov [eax+20], edi       ;

    mov ebx, [esp+36]       ; eax
    mov ecx, [esp+40]       ; eip
    mov esi, [esp+16]       ; ebp
    mov edi, [esp+4]        ; eflags
    mov edx, [esp+20]       ; esp
    add edx, 4              ; Remove the return value

    mov [eax], ebx          ; eax
    mov [eax+24], edx       ; esp
    mov [eax+28], esi       ; ebp
    mov [eax+32], ecx       ; eip
    mov [eax+36], edi       ; eflags

    pop ebx                 ; was pushed last
    mov [eax+40], ebx       ; cr3
    push ebx                ;
 
    mov eax, [esp+48]       ; eax points to "next" task obj

    mov ebx, [eax+4]        ;
    mov ecx, [eax+8]        ;                                     
    mov edx, [eax+12]       ;                                     
    mov esi, [eax+16]       ;                              