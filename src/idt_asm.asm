; idt_asm.asm - Load the IDT pointer register
section .text
global idt_load

idt_load:
    mov eax, [esp + 4]   ; pointer to idt_ptr_t
    lidt [eax]
    ret

; Mark the stack as non-executable
section .note.GNU-stack noalloc noexec nowrite progbits
