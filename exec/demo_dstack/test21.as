g:
mov %r13, %r14
ret

f:
//mov %r11, %r12
//add $8, %r12
add $8, %r11
ret

main:
add $2, %r11
call f
mov %r12, %rax
ret

