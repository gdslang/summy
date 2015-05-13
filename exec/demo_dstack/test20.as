g:
mov %r13, %r14
ret

f:
mov %r11, %r12
ret

main:
mov $99, %r11
call f
mov %r12, %rax
mov $27, %r11
call f
mov %r12, %rbx
ret

