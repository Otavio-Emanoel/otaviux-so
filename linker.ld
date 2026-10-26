/* linker.ld - Linker script for otaviux */

ENTRY(_start)

SECTIONS
{
    /* Kernel is loaded at 1 MB by the bootloader */
    . = 1M;

    .text ALIGN(4K) :
    {
        *(.multiboot)   /* Multiboot header must come first */
        *(.text)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Discard unnecessary sections to keep the kernel small */
    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
}
