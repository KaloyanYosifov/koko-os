[BITS 32]

global _start
global kernel_enable_interrupts
global kernel_disable_interrupts

extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    .load_a20_line:
        in al, 0x92
        or al, 2
        out 0x92, al

    call kernel_remap_master_pic
    call kernel_main

    jmp $

kernel_disable_interrupts:
    cli
    ret

kernel_enable_interrupts:
    sti
    ret

; remap master Programmable Intrerupt Controller (PIC)
kernel_remap_master_pic:
    mov al, 0x11
    out 0x20, al

    mov al, 0x20
    out 0x21, al

    mov al, 0x1
    out 0x21, al

    ret


times 512 - ($ - $$) db 0
