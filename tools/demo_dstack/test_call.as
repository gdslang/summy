.byte 00

f:
mov (%r11), %r12
mov %r12, %rax
ret

main:
movq $42, (%rdx)
mov (%rbx), %rcx
mov %rdx, (%rcx)
mov %rbx, %r11
call f
movq (%rax), %r13
movq (%r13), %r13
ret
