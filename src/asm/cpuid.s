; Utilize 32-bit x86
[BITS 32]

; TODO: can we move this into C code?


; From: http:// ??????

section .bss ; $Revision: 1.6 $
vendor_id       resd	12	;reserve 12 bytes of memory
version		resd	4
features	resd	4
i		resd	4
curfeat		resd	4


section .text
    global _start                       ;must be declared for linker (ld)
names	db	'FPU  VME  DE   PSE  TSC  MSR  PAE  MCE  CX8  APIC RESV SEP  MTRR PGE  MCA  CMOV PAT PSE3 PSN  