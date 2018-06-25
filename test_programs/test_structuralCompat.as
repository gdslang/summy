main:
je else
mov %rbx, (%rax)
jmp end
else:
mov %rcx, (%rax)
end:
ret
