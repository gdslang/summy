f:
mov (%rbx), %r11
mov %r12, (%r11)
movb $22, (%rax)
ret

main:
mov %rcx, %rax
mov %rcx, %rbx
mov %r13, (%rbx)
call f
ret
