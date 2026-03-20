; boot.asm - Multiboot entry point for otaviux
;
; Sets up a minimal 32-bit protected-mode environment, installs a flat GDT,
; and transfers control to kernel_main().

; ─── Multiboot 1 header ──────────────────────────────────────────────────────
MBALIGN  equ 1 << 0
MEMINFO  equ 1 << 1
FLAGS    equ MBALIGN | MEMINFO
MAGIC    equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; ─── Bootstrap stack (16 KB) ─────────────────────────────────────────────────
section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

; ─── GDT (flat 32-bit protected mode) ────────────────────────────────────────
section .data
align 8

gdt_start:
    ; Null descriptor
    dq 0x0000000000000000

    ; Kernel code segment: base=0, limit=4 GB, ring 0, execute/read
    dq 0x00CF9A000000FFFF

    ; Kernel data segment: base=0, limit=4 GB, ring 0, read/write
    dq 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1   ; limit
    dd gdt_start                  ; base

CODE_SEG equ 0x08
DATA_SEG equ 0x10

; ─── Entry point ─────────────────────────────────────────────────────────────
section .text
global _start
extern kernel_main

_start:
    ; 1. Set up the stack
    mov esp, stack_top

    ; 2. Load our own GDT (GRUB may have left a minimal one)
    lgdt [gdt_descriptor]

    ; 3. Reload segment registers with flat data selector
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; 4. Far-jump to reload CS with flat code selector
    jmp CODE_SEG:.flush
.flush:

    ; 5. Clear direction flag and call the C kernel
    cld
    call kernel_main

    ; 6. Should never reach here - freeze the CPU
    cli
.halt:
    hlt
    jmp .halt

; Mark the stack as non-executable
section .note.GNU-stack noalloc noexec nowrite progbits

