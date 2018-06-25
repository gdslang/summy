g:
mov %r13, %r14
ret

f:
mov %rcx, (%rax)
mov %rdx, (%rbx)
je else
mov %rax, %r11
jmp end
else:
mov %rbx, %r11
end:
mov %r11, (%r12)
ret

main:
movq $10, (%rax)
movq $42, (%rbx)
mov %r14, (%r12)
call f
mov (%r11), %r13
ret
