g:
mov %r13, %r14
ret

f:
mov (%rcx), %r15
mov (%rdx), %r15
je else
mov (%rcx), %r11
jmp end
else:
mov (%rdx), %r11
end:
mov %r11, (%r12)
ret

main:
movq $49000, (%r12)
movq $41000, (%rax)
mov %rcx, %rdx
movq %rax, (%rcx)
call f
mov (%r12), %r12
mov (%r12), %r13
ret

