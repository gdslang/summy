main:
//mov %rax, %rax
//mov %rbx, %rbx
//mov %rcx, %rcx
je else
mov %rbx, (%rax)
jmp end
else:
mov %rcx, (%rax)
end:
ret

