; keyboard_asm.asm - Low-level keyboard IRQ stub
;
; The stub saves all registers, calls the C handler, sends EOI, and returns.

section .text
global keyboard_irq_stub
extern keyboard_irq_handler

keyboard_irq_stub:
    pusha
    call keyboard_irq_handler
    popa
    iret

; Mark the stack as non-executable
section .note.GNU-stack noalloc noexec nowrite progbits
