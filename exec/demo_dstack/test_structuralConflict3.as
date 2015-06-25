f:
mov %r11, (%rax)
mov %r12, 4(%rbx)
ret

main:
mov %rcx, %rax
mov %rcx, %rbx
call f
ret
