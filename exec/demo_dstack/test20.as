f:
mov %r11, %r12
ret

main:
mov $99, %r11
call f
mov %r11, %rax
ret

