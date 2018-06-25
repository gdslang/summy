g:
mov %r13, %r14
ret

f:
//add $1, %r12
add $1, %r11
mov %r11, %r12
add $8, %r12
//add $8, %r11
ret

main:
add $2, %r11
call f
mov %r12, %rax
ret

