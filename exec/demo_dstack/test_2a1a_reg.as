g:
mov %r13, %r14
ret

f:
//mov %r11, %r11
mov %rcx, %rcx
mov %rdx, %rdx
je else
mov %rcx, %r11
jmp end
else:
mov %rdx, %r11
end:
mov %r11, (%r12)
ret

main:
movq $41000, (%rcx)
mov %rcx, %rdx
call f
mov (%r12), %r12
mov (%r12), %r13
ret

