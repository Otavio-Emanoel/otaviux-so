# Makefile - Build system for otaviux-so
#
# Requirements:
#   nasm      - assembler (sudo apt install nasm)
#   gcc       - C compiler (sudo apt install gcc-multilib for 32-bit support)
#   ld        - GNU linker
#   grub-mkrescue + xorriso - to create a bootable ISO
#   qemu-system-i386 - to run the OS in emulation

# ─── Toolchain ────────────────────────────────────────────────────────────────
ASM    = nasm
CC     = gcc
LD     = ld
MKISO  = grub-mkrescue
QEMU   = qemu-system-i386

# ─── Flags ────────────────────────────────────────────────────────────────────
ASMFLAGS = -f elf32
CFLAGS   = -m32 -std=c99 -ffreestanding -fno-stack-protector \
            -fno-builtin -nostdlib \
            -Wall -Wextra \
            -Iinclude
LDFLAGS  = -m elf_i386 -T linker.ld --oformat elf32-i386

# ─── Directories ──────────────────────────────────────────────────────────────
SRC_DIR  = src
OBJ_DIR  = obj
ISO_DIR  = isodir
BOOT_DIR = $(ISO_DIR)/boot/grub

# ─── Source files ─────────────────────────────────────────────────────────────
C_SRCS := $(wildcard $(SRC_DIR)/*.c)
ASM_SRCS_SRC := $(wildcard $(SRC_DIR)/*.asm)

C_OBJS   := $(patsubst $(SRC_DIR)/%.c,  $(OBJ_DIR)/%.o, $(C_SRCS))
ASM_OBJS := $(patsubst $(SRC_DIR)/%.asm,$(OBJ_DIR)/%.asm.o, $(ASM_SRCS_SRC))
BOOT_OBJ  = $(OBJ_DIR)/boot.o

OBJS = $(BOOT_OBJ) $(C_OBJS) $(ASM_OBJS)

# ─── Targets ──────────────────────────────────────────────────────────────────
.PHONY: all clean run iso

all: otaviux.elf

# Link everything into the kernel ELF
otaviux.elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo "✔  Kernel ELF: $@"

# Compile boot.asm (root level)
$(BOOT_OBJ): boot.asm | $(OBJ_DIR)
	$(ASM) $(ASMFLAGS) -o $@ $<

# Compile src/*.asm files
$(OBJ_DIR)/%.asm.o: $(SRC_DIR)/%.asm | $(OBJ_DIR)
	$(ASM) $(ASMFLAGS) -o $@ $<

# Compile src/*.c files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ─── Bootable ISO (requires GRUB + xorriso) ───────────────────────────────────
iso: otaviux.elf
	mkdir -p $(BOOT_DIR)
	cp otaviux.elf $(ISO_DIR)/boot/
	printf 'set timeout=0\nset default=0\nmenuentry "otaviux-so" {\n  multiboot /boot/otaviux.elf\n  boot\n}\n' \
	    > $(BOOT_DIR)/grub.cfg
	$(MKISO) -o otaviux.iso $(ISO_DIR)
	@echo "✔  ISO image: otaviux.iso"

# ─── Run in QEMU ──────────────────────────────────────────────────────────────
run: iso
	$(QEMU) -cdrom otaviux.iso -m 32M

# Headless run (for CI / testing)
run-headless: iso
	$(QEMU) -cdrom otaviux.iso -m 32M -nographic -serial mon:stdio

# ─── Clean ────────────────────────────────────────────────────────────────────
clean:
	rm -rf $(OBJ_DIR) $(ISO_DIR) otaviux.elf otaviux.iso
