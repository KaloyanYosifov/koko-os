org 0x7c00
bits 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; used to fix the BPB (BIOS Parameter Block) so that it does not overwrite our code
bpb_fix:
    jmp short preinit
    nop

    ; fill 33 empty bytes to cover for BPB
    times 33 db 0

preinit:
    jmp 0x00:initialize ; sets the code segment to 0x7c0 with the jump, since our origin is now 0

it_handle_zero:
    mov si, it_zero_message
    call print
    iret

initialize:
    cli ; Clear the interrupts
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti ; Enable the interrupts

    ; Fill our Interrupt vector table
    ; The first 2 bytes are the offset
    ; The second 2 bytes are the segment
    ; Example real_address: 0x7c0 * 16 + 14 = 0x7C0E
    ;
    ; We use the stack segment to point to the first address in memory
    ; as it is already initialized by us
    mov word[ss:0x00], it_handle_zero
    mov word[ss:0x02], 0x7c0

    jmp start

start:
    mov si, message
    call print

    jmp initialized_protected_mode

initialized_protected_mode:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp CODE_SEG:load32_mode

print:
    mov ah, 0x0e
    mov bx, 0
    call print_char
    ret

print_char:
    ._loop:
        lodsb
        cmp al, 0
        je ._done
        int 0x10
        jmp ._loop

    ._done:
        ret

gdt_start:
gdt_null:
    dd 0x00
    dd 0x00

; segment descriptor
gdt_code: ; CS(Code segment) point here
    dw 0xffff ;Limit
    dw 0x0000 ;Base
    db 0x00 ;Base
    db 0x9a ;10011010b Access byte
    db 0xCF ;11001111b (Limit, fLag and base)
    db 0x00 ;Base

gdt_data: ; DS, SS, ES, FS, GS point here
    dw 0xffff ;Limit
    dw 0x0000 ;Base
    db 0x00 ;Base
    db 0x92 ;10010010b Access byte
    db 0xCF ;11001111b (Limit, fLag and base)
    db 0x00 ;Base

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
load32_mode:
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

    jmp $

message: db 'Hello World!', 0xa, 0xd, 0
it_zero_message: db 'Cannot divide by 0!', 0xa, 0xd, 0

; we need to fill in 512 bytes for the Master Boot Record
; if we have less we might have an issue with our booting process
; to fix this we fill in with empty instructions sets by subtracting 510 by the amount of bytes/lines we have written before this line
; so if we have filled 50 bytes the code below will fill 460 bytes of empty instructions
times 510 - ($ - $$) db 0
dw 0xAA55
