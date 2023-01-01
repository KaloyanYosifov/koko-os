section .asm

global idt_load
global idt_zero_interrupt
global idt_keyboard_interrupt
global idt_null_interrupt

extern idt_null
extern idt_zero
extern idt_keyboard

idt_load:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]
    lidt [ebx]

    pop ebp
    ret

call_interrupt:
    cli
    pop ecx
    pushad

    call ecx

    popad
    sti
    iret

idt_zero_interrupt:
    mov ecx, idt_zero
    push ecx
    jmp call_interrupt

idt_keyboard_interrupt:
    mov ecx, idt_keyboard
    push ecx
    jmp call_interrupt

idt_null_interrupt:
    mov ecx, idt_null
    push ecx
    jmp call_interrupt
