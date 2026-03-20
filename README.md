# otaviux-so

Sistema operacional desenvolvido em C e Assembly, com interface gráfica funcional em modo texto (VGA 80×25).

## Funcionalidades

- **Boot via Multiboot 1** (compatível com GRUB)
- **Modo protegido 32-bit** com GDT plana
- **Driver VGA texto 80×25** com suporte a cores, cursor de hardware, box-drawing (CP437) e scroll
- **IDT + 8259A PIC** para gerenciamento de interrupções de hardware
- **Driver de teclado PS/2** com mapa US QWERTY, buffer circular e teclas modificadoras
- **Interface gráfica em modo texto**:
  - Barra de título (azul) com nome do SO e versão
  - Área de desktop com fundo pontilhado
  - Barra de tarefas (ciana) com dicas de atalhos
  - Tela de boot splash com logo ASCII
- **Shell interativo** com:
  - Buffer de scrollback de 200 linhas
  - Comandos: `help`, `about`, `clear`, `echo`, `color`, `mem`, `cpu`, `reboot`, `shutdown`, `banner`, `version`

## Estrutura do projeto

```
otaviux-so/
├── boot.asm          # Entry point Multiboot + GDT
├── linker.ld         # Script de linkagem
├── Makefile          # Sistema de build
├── include/          # Cabeçalhos
│   ├── vga.h
│   ├── keyboard.h
│   ├── idt.h
│   ├── pic.h
│   ├── string.h
│   ├── gui.h
│   └── shell.h
└── src/              # Código-fonte C e ASM
    ├── kernel.c       # Ponto de entrada do kernel
    ├── vga.c          # Driver VGA
    ├── string.c       # Utilitários de string (sem libc)
    ├── idt.c / idt_asm.asm        # IDT
    ├── pic.c          # PIC 8259A
    ├── keyboard.c / keyboard_asm.asm  # Driver teclado PS/2
    ├── gui.c          # Componentes GUI
    └── shell.c        # Shell interativo
```

## Como compilar

### Dependências (Ubuntu/Debian)

```bash
sudo apt install nasm gcc-multilib binutils qemu-system-x86 grub-pc-bin xorriso mtools
```

### Build

```bash
make            # Compila o kernel ELF
make iso        # Gera imagem ISO bootável
make run        # Executa no QEMU
make clean      # Limpa artefatos de build
```

