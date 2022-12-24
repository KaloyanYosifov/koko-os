org 0
bits 16

; used to fix the BPB (BIOS Parameter Block) so that it does not overwrite our code
bpb_fix:
    jmp short preinit
    nop

    ; fill 33 empty bytes to cover for BPB
    times 33 db 0

preinit:
    jmp 0x7c0:initialize ; sets the code segment to 0x7c0 with the jump, since our origin is now 0

it_handle_zero:
    mov si, it_zero_message
    call print
    iret

initialize:
    cli ; Clear the interrupts
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    mov ax, 0x00
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
    call read_from_disk

    jmp $

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

hard_disk_error:
    mov si, disk_read_error_message
    call print

    jmp $

; reference --> http://www.ctyme.com/intr/rb-0607.htm
read_from_disk:
    mov ah, 0x02
    mov al, 0x01
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    mov bx, buffer
    int 0x13

    jc hard_disk_error

    mov si, buffer
    call print

    ret

message: db 'Hello World!', 0xa, 0xd, 0
disk_read_error_message: db 'Failed to read from disk!', 0xa, 0xd, 0
it_zero_message: db 'Cannot divide by 0!', 0xa, 0xd, 0

; we need to fill in 512 bytes for the Master Boot Record
; if we have less we might have an issue with our booting process
; to fix this we fill in with empty instructions sets by subtracting 510 by the amount of bytes/lines we have written before this line
; so if we have filled 50 bytes the code below will fill 460 bytes of empty instructions
times 510 - ($ - $$) db 0
dw 0xAA55

buffer:
