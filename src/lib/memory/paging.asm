[BITS 32]

section .asm

global paging_enable_paging
global paging_set_global_page_directory

paging_set_global_page_directory:
    push ebp
    mov ebp, esp

    .set_page_directory:
        mov eax, [ebp+8]
        mov cr3, eax

    pop ebp
    ret

paging_enable_paging:
    push ebp
    mov ebp, esp

    .enable_paging
        mov eax, cr0
        or eax, 0x80000001
        mov cr0, eax

    pop ebp
    ret
