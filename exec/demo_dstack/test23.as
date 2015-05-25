g:
mov %r13, %r14
ret

f:
je else
mov (%rcx), %r11
jmp end
else:
mov (%rdx), %r11
end:
mov %r11, (%r12)
ret

main:
movq $0, (%r12)
mov %rcx, %rdx
movq $42, (%rcx)
call f
mov (%r12), %r12
mov (%r12), %r13
ret

