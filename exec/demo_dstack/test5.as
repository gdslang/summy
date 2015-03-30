jne else
mov %rbx, %rax
jmp end
else:
mov %rcx, %rax
end:
movq $42, (%rax)
ret
