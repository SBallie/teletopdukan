;
; Save for later, or testing

[BITS 32]

extern test_user_function

;--------------------------------------------------------------------

; Load the index of our TSS structure - The index is
; 0x28, as it is the 5th selector and each is 8 bytes
; long, but we set the bottom two bits (making 0x2B)
; so that it has an RPL of 3, not zero.
; Load 0x