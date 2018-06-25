f:
mov (%rax), %r11
mov %r12, (%r11)
movb $22, (%rbx)
ret

main:
mov %rcx, %rax
mov %rcx, %rbx
mov %r13, (%rax)
call f
ret
